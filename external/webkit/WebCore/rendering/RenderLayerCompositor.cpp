/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"

#if USE(ACCELERATED_COMPOSITING)
#include "RenderLayerCompositor.h"

#include "AnimationController.h"
#include "Chrome.h"
#include "ChromeClient.h"
#include "CSSPropertyNames.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsLayer.h"
#include "HitTestResult.h"
#include "HTMLCanvasElement.h"
#include "Page.h"
#include "RenderEmbeddedObject.h"
#include "RenderLayerBacking.h"
#include "RenderReplica.h"
#include "RenderVideo.h"
#include "RenderView.h"
#include "Settings.h"

#if PROFILE_LAYER_REBUILD
#include <wtf/CurrentTime.h>
#endif

#ifndef NDEBUG
#include "CString.h"
#include "RenderTreeAsText.h"
#endif

#if ENABLE(3D_RENDERING)
// This symbol is used to determine from a script whether 3D rendering is enabled (via 'nm').
bool WebCoreHas3DRendering = true;
#endif

namespace WebCore {

using namespace HTMLNames;

struct CompositingState {
    CompositingState(RenderLayer* compAncestor)
        : m_compositingAncestor(compAncestor)
        , m_subtreeIsCompositing(false)
#if ENABLE(COMPOSITED_FIXED_ELEMENTS)
        , m_fixedSibling(false)
#endif
#ifndef NDEBUG
        , m_depth(0)
#endif
    {
    }
    
    RenderLayer* m_compositingAncestor;
    bool m_subtreeIsCompositing;
#if ENABLE(COMPOSITED_FIXED_ELEMENTS)
    bool m_fixedSibling;
#endif
#ifndef NDEBUG
    int m_depth;
#endif
};

RenderLayerCompositor::RenderLayerCompositor(RenderView* renderView)
    : m_renderView(renderView)
    , m_rootPlatformLayer(0)
    , m_hasAcceleratedCompositing(true)
    , m_showDebugBorders(false)
    , m_showRepaintCounter(false)
    , m_compositingConsultsOverlap(true)
    , m_compositing(false)
    , m_rootLayerAttached(false)
    , m_compositingLayersNeedRebuild(false)
#if PROFILE_LAYER_REBUILD
    , m_rootLayerUpdateCount(0)
#endif // PROFILE_LAYER_REBUILD
{
}

RenderLayerCompositor::~RenderLayerCompositor()
{
    ASSERT(!m_rootLayerAttached);
}

void RenderLayerCompositor::enableCompositingMode(bool enable /* = true */)
{
    if (enable != m_compositing) {
        m_compositing = enable;
        
        // We never go out of compositing mode for a given page,
        // but if all the layers disappear, we'll just be left with
        // the empty root layer, which has minimal overhead.
        if (m_compositing)
            ensureRootPlatformLayer();
        else
            destroyRootPlatformLayer();
    }
}

void RenderLayerCompositor::cacheAcceleratedCompositingFlags()
{
    bool hasAcceleratedCompositing = false;
    bool showDebugBorders = false;
    bool showRepaintCounter = false;
    
    if (Settings* settings = m_renderView->document()->settings()) {
        hasAcceleratedCompositing = settings->acceleratedCompositingEnabled();
        showDebugBorders = settings->showDebugBorders();
        showRepaintCounter = settings->showRepaintCounter();
    }

    if (hasAcceleratedCompositing != m_hasAcceleratedCompositing || showDebugBorders != m_showDebugBorders || showRepaintCounter != m_showRepaintCounter)
        setCompositingLayersNeedRebuild();
        
    m_hasAcceleratedCompositing = hasAcceleratedCompositing;
    m_showDebugBorders = showDebugBorders;
    m_showRepaintCounter = showRepaintCounter;
}

void RenderLayerCompositor::setCompositingLayersNeedRebuild(bool needRebuild)
{
    if (inCompositingMode())
        m_compositingLayersNeedRebuild = needRebuild;
}

void RenderLayerCompositor::scheduleSync()
{
    Frame* frame = m_renderView->frameView()->frame();
    Page* page = frame ? frame->page() : 0;
    if (!page)
        return;

    page->chrome()->client()->scheduleCompositingLayerSync();
}

void RenderLayerCompositor::updateCompositingLayers(CompositingUpdateType updateType, RenderLayer* updateRoot)
{
    bool checkForHierarchyUpdate = false;
    bool needGeometryUpdate = false;
    
    switch (updateType) {
    case CompositingUpdateAfterLayoutOrStyleChange:
    case CompositingUpdateOnPaitingOrHitTest:
        checkForHierarchyUpdate = true;
        break;
    case CompositingUpdateOnScroll:
        if (m_compositingConsultsOverlap)
            checkForHierarchyUpdate = true; // Overlap can change with scrolling, so need to check for hierarchy updates.

        needGeometryUpdate = true;
        break;
    }

    if (!checkForHierarchyUpdate && !needGeometryUpdate)
        return;

    ASSERT(inCompositingMode());

    bool needHierarchyUpdate = m_compositingLayersNeedRebuild;
    if (!updateRoot) {
        // Only clear the flag if we're updating the entire hierarchy.
        m_compositingLayersNeedRebuild = false;
        updateRoot = rootRenderLayer();
    }

#if PROFILE_LAYER_REBUILD
    ++m_rootLayerUpdateCount;
    
    double startTime = WTF::currentTime();
#endif        

    if (checkForHierarchyUpdate) {
        // Go through the layers in presentation order, so that we can compute which RenderLayers need compositing layers.
        // FIXME: we could maybe do this and the hierarchy udpate in one pass, but the parenting logic would be more complex.
        CompositingState compState(updateRoot);
        bool layersChanged = false;
        if (m_compositingConsultsOverlap) {
            OverlapMap overlapTestRequestMap;
            computeCompositingRequirements(updateRoot, &overlapTestRequestMap, compState, layersChanged);
        } else
            computeCompositingRequirements(updateRoot, 0, compState, layersChanged);
        
        needHierarchyUpdate |= layersChanged;
    }

    if (needHierarchyUpdate) {
        // Update the hierarchy of the compositing layers.
        CompositingState compState(updateRoot);
        Vector<GraphicsLayer*> childList;
        rebuildCompositingLayerTree(updateRoot, compState, childList);

        // Host the document layer in the RenderView's root layer.
        if (updateRoot == rootRenderLayer() && !childList.isEmpty())
            m_rootPlatformLayer->setChildren(childList);
    } else if (needGeometryUpdate) {
        // We just need to do a geometry update. This is only used for position:fixed scrolling;
        // most of the time, geometry is updated via RenderLayer::styleChanged().
        updateLayerTreeGeometry(updateRoot);
    }
    
#if PROFILE_LAYER_REBUILD
    double endTime = WTF::currentTime();
    if (updateRoot == rootRenderLayer())
        fprintf(stderr, "Update %d: computeCompositingRequirements for the world took %fms\n",
                    m_rootLayerUpdateCount, 1000.0 * (endTime - startTime));
#endif
    ASSERT(updateRoot || !m_compositingLayersNeedRebuild);

    if (!hasAcceleratedCompositing())
        enableCompositingMode(false);
}

bool RenderLayerCompositor::updateBacking(RenderLayer* layer, CompositingChangeRepaint shouldRepaint)
{
    bool layerChanged = false;

    if (needsToBeComposited(layer)) {
        enableCompositingMode();
        
        // 3D transforms turn off the testing of overlap.
        if (requiresCompositingForTransform(layer->renderer()))
            setCompositingConsultsOverlap(false);

        if (!layer->backing()) {

            // If we need to repaint, do so before making backing
            if (shouldRepaint == CompositingChangeRepaintNow)
                repaintOnCompositingChange(layer);

            layer->ensureBacking();
            layerChanged = true;
        }
    } else {
        if (layer->backing()) {
            // If we're removing backing on a reflection, clear the source GraphicsLayer's pointer to
            // its replica GraphicsLayer. In practice this should never happen because reflectee and reflection 
            // are both either composited, or not composited.
            if (layer->isReflection()) {
                RenderLayer* sourceLayer = toRenderBoxModelObject(layer->renderer()->parent())->layer();
                if (RenderLayerBacking* backing = sourceLayer->backing()) {
                    ASSERT(backing->graphicsLayer()->replicaLayer() == layer->backing()->graphicsLayer());
                    backing->graphicsLayer()->setReplicatedByLayer(0);
                }
            }
            
            layer->clearBacking();
            layerChanged = true;

            // The layer's cached repaints rects are relative to the repaint container, so change when
            // compositing changes; we need to update them here.
            layer->computeRepaintRects();

            // If we need to repaint, do so now that we've removed the backing
            if (shouldRepaint == CompositingChangeRepaintNow)
                repaintOnCompositingChange(layer);
        }
    }
    
#if ENABLE(VIDEO)
    if (layerChanged && layer->renderer()->isVideo()) {
        // If it's a video, give the media player a chance to hook up to the layer.
        RenderVideo* video = toRenderVideo(layer->renderer());
        video->acceleratedRenderingStateChanged();
    }
#endif
    return layerChanged;
}

bool RenderLayerCompositor::updateLayerCompositingState(RenderLayer* layer, CompositingChangeRepaint shouldRepaint)
{
    bool layerChanged = updateBacking(layer, shouldRepaint);

    // See if we need content or clipping layers. Methods called here should assume
    // that the compositing state of descendant layers has not been updated yet.
    if (layer->backing() && layer->backing()->updateGraphicsLayerConfiguration())
        layerChanged = true;

    return layerChanged;
}

void RenderLayerCompositor::repaintOnCompositingChange(RenderLayer* layer)
{
    // If the renderer is not attached yet, no need to repaint.
    if (!layer->renderer()->parent())
        return;

    RenderBoxModelObject* repaintContainer = layer->renderer()->containerForRepaint();
    if (!repaintContainer)
        repaintContainer = m_renderView;

    layer->repaintIncludingNonCompositingDescendants(repaintContainer);
    if (repaintContainer == m_renderView) {
        // The contents of this layer may be moving between the window
        // and a GraphicsLayer, so we need to make sure the window system
        // synchronizes those changes on the screen.
        m_renderView->frameView()->setNeedsOneShotDrawingSynchronization();
    }
}

// The bounds of the GraphicsLayer created for a compositing layer is the union of the bounds of all the descendant
// RenderLayers that are rendered by the composited RenderLayer.
IntRect RenderLayerCompositor::calculateCompositedBounds(const RenderLayer* layer, const RenderLayer* ancestorLayer)
{
    if (!layer->isSelfPaintingLayer())
        return IntRect();

    IntRect boundingBoxRect, unionBounds;
    boundingBoxRect = unionBounds = layer->localBoundingBox();
    
    if (layer->renderer()->hasOverflowClip() || layer->renderer()->hasMask()) {
        int ancestorRelX = 0, ancestorRelY = 0;
        layer->convertToLayerCoords(ancestorLayer, ancestorRelX, ancestorRelY);
        boundingBoxRect.move(ancestorRelX, ancestorRelY);
        return boundingBoxRect;
    }

    if (RenderLayer* reflection = layer->reflectionLayer()) {
        if (!reflection->isComposited()) {
            IntRect childUnionBounds = calculateCompositedBounds(reflection, layer);
            unionBounds.unite(childUnionBounds);
        }
    }
    
    ASSERT(layer->isStackingContext() || (!layer->m_posZOrderList || layer->m_posZOrderList->size() == 0));

    if (Vector<RenderLayer*>* negZOrderList = layer->negZOrderList()) {
        size_t listSize = negZOrderList->size();
        for (size_t i = 0; i < listSize; ++i) {
            RenderLayer* curLayer = negZOrderList->at(i);
            if (!curLayer->isComposited()) {
                IntRect childUnionBounds = calculateCompositedBounds(curLayer, layer);
                unionBounds.unite(childUnionBounds);
            }
        }
    }

    if (Vector<RenderLayer*>* posZOrderList = layer->posZOrderList()) {
        size_t listSize = posZOrderList->size();
        for (size_t i = 0; i < listSize; ++i) {
            RenderLayer* curLayer = posZOrderList->at(i);
            if (!curLayer->isComposited()) {
                IntRect childUnionBounds = calculateCompositedBounds(curLayer, layer);
                unionBounds.unite(childUnionBounds);
            }
        }
    }

    if (Vector<RenderLayer*>* normalFlowList = layer->normalFlowList()) {
        size_t listSize = normalFlowList->size();
        for (size_t i = 0; i < listSize; ++i) {
            RenderLayer* curLayer = normalFlowList->at(i);
            if (!curLayer->isComposited()) {
                IntRect curAbsBounds = calculateCompositedBounds(curLayer, layer);
                unionBounds.unite(curAbsBounds);
            }
        }
    }

    if (layer->paintsWithTransform(PaintBehaviorNormal)) {
        TransformationMatrix* affineTrans = layer->transform();
        boundingBoxRect = affineTrans->mapRect(boundingBoxRect);
        unionBounds = affineTrans->mapRect(unionBounds);
    }

    int ancestorRelX = 0, ancestorRelY = 0;
    layer->convertToLayerCoords(ancestorLayer, ancestorRelX, ancestorRelY);
    unionBounds.move(ancestorRelX, ancestorRelY);

    return unionBounds;
}

void RenderLayerCompositor::layerWasAdded(RenderLayer* /*parent*/, RenderLayer* /*child*/)
{
    setCompositingLayersNeedRebuild();
}

void RenderLayerCompositor::layerWillBeRemoved(RenderLayer* parent, RenderLayer* child)
{
    if (!child->isComposited() || parent->renderer()->documentBeingDestroyed())
        return;

    setCompositingParent(child, 0);
    
    RenderLayer* compLayer = parent->enclosingCompositingLayer();
    if (compLayer) {
        ASSERT(compLayer->backing());
        IntRect compBounds = child->backing()->compositedBounds();

        int offsetX = 0, offsetY = 0;
        child->convertToLayerCoords(compLayer, offsetX, offsetY);
        compBounds.move(offsetX, offsetY);

        compLayer->setBackingNeedsRepaintInRect(compBounds);

        // The contents of this layer may be moving from a GraphicsLayer to the window,
        // so we need to make sure the window system synchronizes those changes on the screen.
        m_renderView->frameView()->setNeedsOneShotDrawingSynchronization();
    }

    setCompositingLayersNeedRebuild();
}

RenderLayer* RenderLayerCompositor::enclosingNonStackingClippingLayer(const RenderLayer* layer) const
{
    for (RenderLayer* curr = layer->parent(); curr != 0; curr = curr->parent()) {
        if (curr->isStackingContext())
            return 0;

        if (curr->renderer()->hasOverflowClip())
            return curr;
    }
    return 0;
}

void RenderLayerCompositor::addToOverlapMap(OverlapMap& overlapMap, RenderLayer* layer, IntRect& layerBounds, bool& boundsComputed)
{
    if (layer->isRootLayer())
        return;

    if (!boundsComputed) {
        layerBounds = layer->renderer()->localToAbsoluteQuad(FloatRect(layer->localBoundingBox())).enclosingBoundingBox();
        boundsComputed = true;
    }

    overlapMap.add(layer, layerBounds);
}

bool RenderLayerCompositor::overlapsCompositedLayers(OverlapMap& overlapMap, const IntRect& layerBounds)
{
    RenderLayerCompositor::OverlapMap::const_iterator end = overlapMap.end();
    for (RenderLayerCompositor::OverlapMap::const_iterator it = overlapMap.begin(); it != end; ++it) {
        const IntRect& bounds = it->second;
        if (layerBounds.intersects(bounds))
            return true;
    }
    
    return false;
}

//  Recurse through the layers in z-index and overflow order (which is equivalent to painting order)
//  For the z-order children of a compositing layer:
//      If a child layers has a compositing layer, then all subsequent layers must
//      be compositing in order to render above that layer.
//
//      If a child in the negative z-order list is compositing, then the layer itself
//      must be compositing so that its contents render over that child.
//      This implies that its positive z-index children must also be compositing.
//
void RenderLayerCompositor::computeCompositingRequirements(RenderLayer* layer, OverlapMap* overlapMap, struct CompositingState& compositingState, bool& layersChanged)
{
    layer->updateLayerPosition();
    layer->updateZOrderLists();
    layer->updateNormalFlowList();
    
    // Clear the flag
    layer->setHasCompositingDescendant(false);
    
    bool mustOverlapCompositedLayers = compositingState.m_subtreeIsCompositing;

    bool haveComputedBounds = false;
    IntRect absBounds;
    if (overlapMap && !overlapMap->isEmpty()) {
        // If we're testing for overlap, we only need to composite if we overlap something that is already composited.
        absBounds = layer->renderer()->localToAbsoluteQuad(FloatRect(layer->localBoundingBox())).enclosingBoundingBox();
        haveComputedBounds = true;
        mustOverlapCompositedLayers = overlapsCompositedLayers(*overlapMap, absBounds);
    }
    
    layer->setMustOverlapCompositedLayers(mustOverlapCompositedLayers);
    
    // The children of this layer don't need to composite, unless there is
    // a compositing layer among them, so start by inheriting the compositing
    // ancestor with m_subtreeIsCompositing set to false.
    CompositingState childState(compositingState.m_compositingAncestor);
#ifndef NDEBUG
    ++childState.m_depth;
#endif

    const bool willBeComposited = needsToBeComposited(layer);

#if ENABLE(COMPOSITED_FIXED_ELEMENTS)
    // If we are a fixed layer, signal it to our siblings
    if (layer->isFixed())
        compositingState.m_fixedSibling = true;

    if (!willBeComposited && compositingState.m_fixedSibling)
        layer->setMustOverlapCompositedLayers(true);

    if (willBeComposited || compositingState.m_fixedSibling) {
#else
    if (willBeComposited) {
#endif
        // Tell the parent it has compositing descendants.
        compositingState.m_subtreeIsCompositing = true;
        // This layer now acts as the ancestor for kids.
        childState.m_compositingAncestor = layer;
        if (overlapMap)
            addToOverlapMap(*overlapMap, layer, absBounds, haveComputedBounds);
    }

#if ENABLE(VIDEO)
    // Video is special. It's a replaced element with a content layer, but has shadow content
    // for the controller that must render in front. Without this, the controls fail to show
    // when the video element is a stacking context (e.g. due to opacity or transform).
    if (willBeComposited && layer->renderer()->isVideo())
        childState.m_subtreeIsCompositing = true;
#endif

    if (layer->isStackingContext()) {
        ASSERT(!layer->m_zOrderListsDirty);
        if (Vector<RenderLayer*>* negZOrderList = layer->negZOrderList()) {
            size_t listSize = negZOrderList->size();
#if ENABLE(COMPOSITED_FIXED_ELEMENTS)
            childState.m_fixedSibling = false;

            // For the negative z-order, if we have a fixed layer
            // we need to make all the siblings composited layers.
            // Otherwise a negative layer (below the fixed layer) could
            // still be drawn onto a higher z-order layer (e.g. the body)
            // if not immediately intersecting with our fixed layer.
            // So it's not enough here to only set m_fixedSibling for
            // subsequent siblings as we do for the normal flow
            // and positive z-order.
            for (size_t j = 0; j < listSize; ++j) {
                if ((negZOrderList->at(j))->isFixed()) {
                    childState.m_fixedSibling = true;
                    break;
                }
            }
#endif

            for (size_t i = 0; i < listSize; ++i) {
                RenderLayer* curLayer = negZOrderList->at(i);
                computeCompositingRequirements(curLayer, overlapMap, childState, layersChanged);

                // If we have to make a layer for this child, make one now so we can have a contents layer
                // (since we need to ensure that the -ve z-order child renders underneath our contents).
                if (!willBeComposited && childState.m_subtreeIsCompositing) {
                    // make layer compositing
                    layer->setMustOverlapCompositedLayers(true);
                    childState.m_compositingAncestor = layer;
                    if (overlapMap)
                        addToOverlapMap(*overlapMap, layer, absBounds, haveComputedBounds);
                }
            }
        }
    }
    
    ASSERT(!layer->m_normalFlowListDirty);
    if (Vector<RenderLayer*>* normalFlowList = layer->normalFlowList()) {
        size_t listSize = normalFlowList->size();
#if ENABLE(COMPOSITED_FIXED_ELEMENTS)
        childState.m_fixedSibling = false;
#endif
        for (size_t i = 0; i < listSize; ++i) {
            RenderLayer* curLayer = normalFlowList->at(i);
            computeCompositingRequirements(curLayer, overlapMap, childState, layersChanged);
        }
    }

    if (layer->isStackingContext()) {
        if (Vector<RenderLayer*>* posZOrderList = layer->posZOrderList()) {
            size_t listSize = posZOrderList->size();
#if ENABLE(COMPOSITED_FIXED_ELEMENTS)
            childState.m_fixedSibling = false;
#endif
            for (size_t i = 0; i < listSize; ++i) {
                RenderLayer* curLayer = posZOrderList->at(i);
                computeCompositingRequirements(curLayer, overlapMap, childState, layersChanged);
            }
        }
    }

    // If we have a software transform, and we have layers under us, we need to also
    // be composited. Also, if we have opacity < 1, then we need to be a layer so that
    // the child layers are opaque, then rendered with opacity on this layer.
    if (!willBeComposited && childState.m_subtreeIsCompositing && requiresCompositingWhenDescendantsAreCompositing(layer->renderer())) {
        layer->setMustOverlapCompositedLayers(true);
        if (overlapMap)
            addToOverlapMap(*overlapMap, layer, absBounds, haveComputedBounds);
    }

    if (layer->reflectionLayer())
        layer->reflectionLayer()->setMustOverlapCompositedLayers(needsToBeComposited(layer));

    // Subsequent layers in the parent stacking context also need to composite.
    if (childState.m_subtreeIsCompositing)
        compositingState.m_subtreeIsCompositing = true;

    // If the layer is going into compositing mode, repaint its old location.
    if (!layer->isComposited() && needsToBeComposited(layer))
        repaintOnCompositingChange(layer);

    // Set the flag to say that this SC has compositing children.
    // this can affect the answer to needsToBeComposited() when clipping,
    // but that's ok here.
    layer->setHasCompositingDescendant(childState.m_subtreeIsCompositing);

    // Update backing now, so that we can use isComposited() reliably during tree traversal in rebuildCompositingLayerTree().
    if (updateBacking(layer, CompositingChangeRepaintNow))
        layersChanged = true;

    if (layer->reflectionLayer() && updateLayerCompositingState(layer->reflectionLayer(), CompositingChangeRepaintNow))
        layersChanged = true;
}

void RenderLayerCompositor::setCompositingParent(RenderLayer* childLayer, RenderLayer* parentLayer)
{
    ASSERT(!parentLayer || childLayer->ancestorCompositingLayer() == parentLayer);
    ASSERT(childLayer->isComposited());

    // It's possible to be called with a parent that isn't yet composited when we're doing
    // partial updates as required by painting or hit testing. Just bail in that case;
    // we'll do a full layer update soon.
    if (!parentLayer || !parentLayer->isComposited())
        return;

    if (parentLayer) {
        GraphicsLayer* hostingLayer = parentLayer->backing()->parentForSublayers();
        GraphicsLayer* hostedLayer = childLayer->backing()->childForSuperlayers();
        
        hostingLayer->addChild(hostedLayer);
    } else
        childLayer->backing()->childForSuperlayers()->removeFromParent();
}

void RenderLayerCompositor::removeCompositedChildren(RenderLayer* layer)
{
    ASSERT(layer->isComposited());

    GraphicsLayer* hostingLayer = layer->backing()->parentForSublayers();
    hostingLayer->removeAllChildren();
}

void RenderLayerCompositor::parentInRootLayer(RenderLayer* layer)
{
    ASSERT(layer->isComposited());

    GraphicsLayer* layerAnchor = layer->backing()->childForSuperlayers();

    if (layerAnchor->parent() != m_rootPlatformLayer) {
        layerAnchor->removeFromParent();
        if (m_rootPlatformLayer)
            m_rootPlatformLayer->addChild(layerAnchor);
    }
}

#if ENABLE(VIDEO)
bool RenderLayerCompositor::canAccelerateVideoRendering(RenderVideo* o) const
{
    if (!m_hasAcceleratedCompositing)
        return false;

    return o->supportsAcceleratedRendering();
}
#endif

void RenderLayerCompositor::rebuildCompositingLayerTree(RenderLayer* layer, const CompositingState& compositingState, Vector<GraphicsLayer*>& childLayersOfEnclosingLayer)
{
    // Make the layer compositing if necessary, and set up clipping and content layers.
    // Note that we can only do work here that is independent of whether the descendant layers
    // have been processed. computeCompositingRequirements() will already have done the repaint if necessary.
    
    RenderLayerBacking* layerBacking = layer->backing();
    if (layerBacking) {
        // The compositing state of all our children has been updated already, so now
        // we can compute and cache the composited bounds for this layer.
        layerBacking->updateCompositedBounds();

        if (RenderLayer* reflection = layer->reflectionLayer()) {
            if (reflection->backing())
                reflection->backing()->updateCompositedBounds();
        }

        layerBacking->updateGraphicsLayerConfiguration();
        layerBacking->updateGraphicsLayerGeometry();

        if (!layer->parent())
            updateRootLayerPosition();
    }

    // If this layer has backing, then we are collecting its children, otherwise appending
    // to the compositing child list of an enclosing layer.
    Vector<GraphicsLayer*> layerChildren;
    Vector<GraphicsLayer*>& childList = layerBacking ? layerChildren : childLayersOfEnclosingLayer;

    CompositingState childState = compositingState;
    if (layer->isComposited())
        childState.m_compositingAncestor = layer;

#ifndef NDEBUG
    ++childState.m_depth;
#endif

    // The children of this stacking context don't need to composite, unless there is
    // a compositing layer among them, so start by assuming false.
    childState.m_subtreeIsCompositing = false;

    if (layer->isStackingContext()) {
        ASSERT(!layer->m_zOrderListsDirty);

        if (Vector<RenderLayer*>* negZOrderList = layer->negZOrderList()) {
            size_t listSize = negZOrderList->size();
            for (size_t i = 0; i < listSize; ++i) {
                RenderLayer* curLayer = negZOrderList->at(i);
                rebuildCompositingLayerTree(curLayer, childState, childList);
            }
        }

        // If a negative z-order child is compositing, we get a foreground layer which needs to get parented.
        if (layerBacking && layerBacking->foregroundLayer())
            childList.append(layerBacking->foregroundLayer());
    }

    ASSERT(!layer->m_normalFlowListDirty);
    if (Vector<RenderLayer*>* normalFlowList = layer->normalFlowList()) {
        size_t listSize = normalFlowList->size();
        for (size_t i = 0; i < listSize; ++i) {
            RenderLayer* curLayer = normalFlowList->at(i);
            rebuildCompositingLayerTree(curLayer, childState, childList);
        }
    }

    if (layer->isStackingContext()) {
        if (Vector<RenderLayer*>* posZOrderList = layer->posZOrderList()) {
            size_t listSize = posZOrderList->size();
            for (size_t i = 0; i < listSize; ++i) {
                RenderLayer* curLayer = posZOrderList->at(i);
                rebuildCompositingLayerTree(curLayer, childState, childList);
            }
        }
    }

    if (layerBacking) {
        layerBacking->parentForSublayers()->setChildren(layerChildren);
        childLayersOfEnclosingLayer.append(layerBacking->childForSuperlayers());
    }
}

// This just updates layer geometry without changing the hierarchy.
void RenderLayerCompositor::updateLayerTreeGeometry(RenderLayer* layer)
{
    if (RenderLayerBacking* layerBacking = layer->backing()) {
        // The compositing state of all our children has been updated already, so now
        // we can compute and cache the composited bounds for this layer.
        layerBacking->updateCompositedBounds();

        if (RenderLayer* reflection = layer->reflectionLayer()) {
            if (reflection->backing())
                reflection->backing()->updateCompositedBounds();
        }

        layerBacking->updateGraphicsLayerConfiguration();
        layerBacking->updateGraphicsLayerGeometry();

        if (!layer->parent())
            updateRootLayerPosition();
    }

    if (layer->isStackingContext()) {
        ASSERT(!layer->m_zOrderListsDirty);

        if (Vector<RenderLayer*>* negZOrderList = layer->negZOrderList()) {
            size_t listSize = negZOrderList->size();
            for (size_t i = 0; i < listSize; ++i)
                updateLayerTreeGeometry(negZOrderList->at(i));
        }
    }

    ASSERT(!layer->m_normalFlowListDirty);
    if (Vector<RenderLayer*>* normalFlowList = layer->normalFlowList()) {
        size_t listSize = normalFlowList->size();
        for (size_t i = 0; i < listSize; ++i)
            updateLayerTreeGeometry(normalFlowList->at(i));
    }
    
    if (layer->isStackingContext()) {
        if (Vector<RenderLayer*>* posZOrderList = layer->posZOrderList()) {
            size_t listSize = posZOrderList->size();
            for (size_t i = 0; i < listSize; ++i)
                updateLayerTreeGeometry(posZOrderList->at(i));
        }
    }
}

// Recurs down the RenderLayer tree until its finds the compositing descendants of compositingAncestor and updates their geometry.
void RenderLayerCompositor::updateCompositingDescendantGeometry(RenderLayer* compositingAncestor, RenderLayer* layer, RenderLayerBacking::UpdateDepth updateDepth)
{
    if (layer != compositingAncestor) {
        if (RenderLayerBacking* layerBacking = layer->backing()) {
            layerBacking->updateCompositedBounds();

            if (RenderLayer* reflection = layer->reflectionLayer()) {
                if (reflection->backing())
                    reflection->backing()->updateCompositedBounds();
            }

            layerBacking->updateGraphicsLayerGeometry();
            if (updateDepth == RenderLayerBacking::CompositingChildren)
                return;
        }
    }

    if (layer->reflectionLayer())
        updateCompositingDescendantGeometry(compositingAncestor, layer->reflectionLayer(), updateDepth);

    if (!layer->hasCompositingDescendant())
        return;
    
    if (layer->isStackingContext()) {
        if (Vector<RenderLayer*>* negZOrderList = layer->negZOrderList()) {
            size_t listSize = negZOrderList->size();
            for (size_t i = 0; i < listSize; ++i)
                updateCompositingDescendantGeometry(compositingAncestor, negZOrderList->at(i), updateDepth);
        }
    }

    if (Vector<RenderLayer*>* normalFlowList = layer->normalFlowList()) {
        size_t listSize = normalFlowList->size();
        for (size_t i = 0; i < listSize; ++i)
            updateCompositingDescendantGeometry(compositingAncestor, normalFlowList->at(i), updateDepth);
    }
    
    if (layer->isStackingContext()) {
        if (Vector<RenderLayer*>* posZOrderList = layer->posZOrderList()) {
            size_t listSize = posZOrderList->size();
            for (size_t i = 0; i < listSize; ++i)
                updateCompositingDescendantGeometry(compositingAncestor, posZOrderList->at(i), updateDepth);
        }
    }
}


void RenderLayerCompositor::repaintCompositedLayersAbsoluteRect(const IntRect& absRect)
{
    recursiveRepaintLayerRect(rootRenderLayer(), absRect);
}

void RenderLayerCompositor::recursiveRepaintLayerRect(RenderLayer* layer, const IntRect& rect)
{
    // FIXME: This method does not work correctly with transforms.
    if (layer->isComposited())
        layer->setBackingNeedsRepaintInRect(rect);

    if (layer->hasCompositingDescendant()) {
        if (Vector<RenderLayer*>* negZOrderList = layer->negZOrderList()) {
            size_t listSize = negZOrderList->size();
            for (size_t i = 0; i < listSize; ++i) {
                RenderLayer* curLayer = negZOrderList->at(i);
                int x = 0;
                int y = 0;
                curLayer->convertToLayerCoords(layer, x, y);
                IntRect childRect(rect);
                childRect.move(-x, -y);
                recursiveRepaintLayerRect(curLayer, childRect);
            }
        }

        if (Vector<RenderLayer*>* posZOrderList = layer->posZOrderList()) {
            size_t listSize = posZOrderList->size();
            for (size_t i = 0; i < listSize; ++i) {
                RenderLayer* curLayer = posZOrderList->at(i);
                int x = 0;
                int y = 0;
                curLayer->convertToLayerCoords(layer, x, y);
                IntRect childRect(rect);
                childRect.move(-x, -y);
                recursiveRepaintLayerRect(curLayer, childRect);
            }
        }
    }
    if (Vector<RenderLayer*>* normalFlowList = layer->normalFlowList()) {
        size_t listSize = normalFlowList->size();
        for (size_t i = 0; i < listSize; ++i) {
            RenderLayer* curLayer = normalFlowList->at(i);
            int x = 0;
            int y = 0;
            curLayer->convertToLayerCoords(layer, x, y);
            IntRect childRect(rect);
            childRect.move(-x, -y);
            recursiveRepaintLayerRect(curLayer, childRect);
        }
    }
}

RenderLayer* RenderLayerCompositor::rootRenderLayer() const
{
    return m_renderView->layer();
}

GraphicsLayer* RenderLayerCompositor::rootPlatformLayer() const
{
    return m_rootPlatformLayer.get();
}

void RenderLayerCompositor::didMoveOnscreen()
{
    if (!m_rootPlatformLayer)
        return;

    Frame* frame = m_renderView->frameView()->frame();
    Page* page = frame ? frame->page() : 0;
    if (!page)
        return;

    page->chrome()->client()->attachRootGraphicsLayer(frame, m_rootPlatformLayer.get());
    m_rootLayerAttached = true;
}

void RenderLayerCompositor::willMoveOffscreen()
{
    if (!m_rootPlatformLayer || !m_rootLayerAttached)
        return;

    Frame* frame = m_renderView->frameView()->frame();
    Page* page = frame ? frame->page() : 0;
    if (!page)
        return;

    page->chrome()->client()->attachRootGraphicsLayer(frame, 0);
    m_rootLayerAttached = false;
}

void RenderLayerCompositor::updateRootLayerPosition()
{
    if (m_rootPlatformLayer)
        m_rootPlatformLayer->setSize(FloatSize(m_renderView->rightLayoutOverflow(), m_renderView->bottomLayoutOverflow()));
}

void RenderLayerCompositor::didStartAcceleratedAnimation()
{
    // If an accelerated animation or transition runs, we have to turn off overlap checking because
    // we don't do layout for every frame, but we have to ensure that the layering is
    // correct between the animating object and other objects on the page.
    setCompositingConsultsOverlap(false);
}

bool RenderLayerCompositor::has3DContent() const
{
    return layerHas3DContent(rootRenderLayer());
}

bool RenderLayerCompositor::needsToBeComposited(const RenderLayer* layer) const
{
    if (!m_hasAcceleratedCompositing || !layer->isSelfPaintingLayer())
        return false;

#if ENABLE(COMPOSITED_FIXED_ELEMENTS)
    // if an ancestor is fixed positioned, we need to be composited...
    const RenderLayer* currLayer = layer;
    while ((currLayer = currLayer->parent())) {
        if (currLayer->isComposited() && currLayer->isFixed())
            return true;
    }
#endif

    return requiresCompositingLayer(layer) || layer->mustOverlapCompositedLayers();
}

#if PLATFORM(ANDROID)
bool RenderLayerCompositor::requiresCompositingForMobileSites(const RenderLayer* layer) const
{
    // First, check if we are in an iframe, and if so bail out
    if (m_renderView->document()->frame()->tree()->parent())
        return false;

    RenderObject* renderer = layer->renderer();
    // Check for transforms
    if (requiresCompositingForTransform(renderer))
        return true;

    // Check for animations
    if (requiresCompositingForAnimation(renderer))
        return true;

#if ENABLE(COMPOSITED_FIXED_ELEMENTS)
    // For the moment, we want to only enable fixed composited layers on mobile websites.
    // We can consider a website as being a 'mobile' site if all the
    // following checks are true:
    // 1) - the viewport width is either undefined (-1) or equal to device-width (0), and
    // 2) - no scaling is allowed
    if (!layer->isFixed())
        return false;

    Settings* settings = m_renderView->document()->settings();
    if (!settings)
        return false;

    if ((settings->viewportWidth() == -1 || settings->viewportWidth() == 0) &&
        !settings->viewportUserScalable())
        return true;
#endif

    return false;
}
#endif

// Note: this specifies whether the RL needs a compositing layer for intrinsic reasons.
// Use needsToBeComposited() to determine if a RL actually needs a compositing layer.
// static
bool RenderLayerCompositor::requiresCompositingLayer(const RenderLayer* layer) const
{
    RenderObject* renderer = layer->renderer();
    // The compositing state of a reflection should match that of its reflected layer.
    if (layer->isReflection()) {
        renderer = renderer->parent(); // The RenderReplica's parent is the object being reflected.
        layer = toRenderBoxModelObject(renderer)->layer();
    }
    // The root layer always has a compositing layer, but it may not have backing.
    return (inCompositingMode() && layer->isRootLayer()) ||
#if PLATFORM(ANDROID)
             requiresCompositingForMobileSites(layer) ||
#else
             requiresCompositingForTransform(renderer) ||
             requiresCompositingForVideo(renderer) ||
             requiresCompositingForCanvas(renderer) ||
             requiresCompositingForPlugin(renderer) ||
             requiresCompositingForAnimation(renderer) ||
#endif
             renderer->style()->backfaceVisibility() == BackfaceVisibilityHidden ||
             clipsCompositingDescendants(layer);
}

// Return true if the given layer has some ancestor in the RenderLayer hierarchy that clips,
// up to the enclosing compositing ancestor. This is required because compositing layers are parented
// according to the z-order hierarchy, yet clipping goes down the renderer hierarchy.
// Thus, a RenderLayer can be clipped by a RenderLayer that is an ancestor in the renderer hierarchy,
// but a sibling in the z-order hierarchy.
bool RenderLayerCompositor::clippedByAncestor(RenderLayer* layer) const
{
    if (!layer->isComposited() || !layer->parent())
        return false;

    RenderLayer* compositingAncestor = layer->ancestorCompositingLayer();
    if (!compositingAncestor)
        return false;

    // If the compositingAncestor clips, that will be taken care of by clipsCompositingDescendants(),
    // so we only care about clipping between its first child that is our ancestor (the computeClipRoot),
    // and layer.
    RenderLayer* computeClipRoot = 0;
    RenderLayer* curr = layer;
    while (curr) {
        RenderLayer* next = curr->parent();
        if (next == compositingAncestor) {
            computeClipRoot = curr;
            break;
        }
        curr = next;
    }
    
    if (!computeClipRoot || computeClipRoot == layer)
        return false;

    IntRect backgroundRect = layer->backgroundClipRect(computeClipRoot, true);
    return backgroundRect != ClipRects::infiniteRect();
}

// Return true if the given layer is a stacking context and has compositing child
// layers that it needs to clip. In this case we insert a clipping GraphicsLayer
// into the hierarchy between this layer and its children in the z-order hierarchy.
bool RenderLayerCompositor::clipsCompositingDescendants(const RenderLayer* layer) const
{
    // FIXME: need to look at hasClip() too eventually
    return layer->hasCompositingDescendant() &&
           layer->renderer()->hasOverflowClip();
}

bool RenderLayerCompositor::requiresCompositingForTransform(RenderObject* renderer) const
{
    RenderStyle* style = renderer->style();
    // Note that we ask the renderer if it has a transform, because the style may have transforms,
    // but the renderer may be an inline that doesn't suppport them.
    return renderer->hasTransform() && (style->transform().has3DOperation() || style->transformStyle3D() == TransformStyle3DPreserve3D || style->hasPerspective());
}

bool RenderLayerCompositor::requiresCompositingForVideo(RenderObject* renderer) const
{
#if ENABLE(VIDEO)
    if (renderer->isVideo()) {
        RenderVideo* video = toRenderVideo(renderer);
        return canAccelerateVideoRendering(video);
    }
#else
    UNUSED_PARAM(renderer);
#endif
    return false;
}

bool RenderLayerCompositor::requiresCompositingForCanvas(RenderObject* renderer) const
{
#if ENABLE(3D_CANVAS)    
    if (renderer->isCanvas()) {
        HTMLCanvasElement* canvas = static_cast<HTMLCanvasElement*>(renderer->node());
        return canvas->is3D();
    }
#else
    UNUSED_PARAM(renderer);
#endif
    return false;
}

bool RenderLayerCompositor::requiresCompositingForPlugin(RenderObject* renderer) const
{
    return renderer->isEmbeddedObject() && toRenderEmbeddedObject(renderer)->allowsAcceleratedCompositing();
}

bool RenderLayerCompositor::requiresCompositingForAnimation(RenderObject* renderer) const
{
    if (AnimationController* animController = renderer->animation()) {
        return (animController->isAnimatingPropertyOnRenderer(renderer, CSSPropertyOpacity) && inCompositingMode())
            || animController->isAnimatingPropertyOnRenderer(renderer, CSSPropertyWebkitTransform);
    }
    return false;
}

bool RenderLayerCompositor::requiresCompositingWhenDescendantsAreCompositing(RenderObject* renderer) const
{
    return renderer->hasTransform() || renderer->isTransparent() || renderer->hasMask() || renderer->hasReflection();
}

// If an element has negative z-index children, those children render in front of the 
// layer background, so we need an extra 'contents' layer for the foreground of the layer
// object.
bool RenderLayerCompositor::needsContentsCompositingLayer(const RenderLayer* layer) const
{
    return (layer->m_negZOrderList && layer->m_negZOrderList->size() > 0);
}

void RenderLayerCompositor::ensureRootPlatformLayer()
{
    if (m_rootPlatformLayer)
        return;

    m_rootPlatformLayer = GraphicsLayer::create(0);
    m_rootPlatformLayer->setSize(FloatSize(m_renderView->rightLayoutOverflow(), m_renderView->bottomLayoutOverflow()));
    m_rootPlatformLayer->setPosition(FloatPoint(0, 0));
    // The root layer does flipping if we need it on this platform.
    m_rootPlatformLayer->setGeometryOrientation(GraphicsLayer::compositingCoordinatesOrientation());

    // Need to clip to prevent transformed content showing outside this frame
    m_rootPlatformLayer->setMasksToBounds(true);
    
    didMoveOnscreen();
}

void RenderLayerCompositor::destroyRootPlatformLayer()
{
    if (!m_rootPlatformLayer)
        return;

    willMoveOffscreen();
    m_rootPlatformLayer = 0;
}

bool RenderLayerCompositor::layerHas3DContent(const RenderLayer* layer) const
{
    const RenderStyle* style = layer->renderer()->style();

    if (style && 
        (style->transformStyle3D() == TransformStyle3DPreserve3D ||
         style->hasPerspective() ||
         style->transform().has3DOperation()))
        return true;

    if (layer->isStackingContext()) {
        if (Vector<RenderLayer*>* negZOrderList = layer->negZOrderList()) {
            size_t listSize = negZOrderList->size();
            for (size_t i = 0; i < listSize; ++i) {
                RenderLayer* curLayer = negZOrderList->at(i);
                if (layerHas3DContent(curLayer))
                    return true;
            }
        }

        if (Vector<RenderLayer*>* posZOrderList = layer->posZOrderList()) {
            size_t listSize = posZOrderList->size();
            for (size_t i = 0; i < listSize; ++i) {
                RenderLayer* curLayer = posZOrderList->at(i);
                if (layerHas3DContent(curLayer))
                    return true;
            }
        }
    }

    if (Vector<RenderLayer*>* normalFlowList = layer->normalFlowList()) {
        size_t listSize = normalFlowList->size();
        for (size_t i = 0; i < listSize; ++i) {
            RenderLayer* curLayer = normalFlowList->at(i);
            if (layerHas3DContent(curLayer))
                return true;
        }
    }
    return false;
}

} // namespace WebCore

#endif // USE(ACCELERATED_COMPOSITING)
