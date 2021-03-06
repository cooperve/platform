/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2007 David Smith (catfish.man@gmail.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef RenderBlock_h
#define RenderBlock_h

#include "DeprecatedPtrList.h"
#include "GapRects.h"
#include "RenderBox.h"
#include "RenderLineBoxList.h"
#include "RootInlineBox.h"
#include <wtf/ListHashSet.h>

namespace WebCore {

class InlineIterator;
class RenderInline;

struct BidiRun;

template <class Iterator, class Run> class BidiResolver;
template <class Iterator> class MidpointState;
typedef BidiResolver<InlineIterator, BidiRun> InlineBidiResolver;
typedef MidpointState<InlineIterator> LineMidpointState;

enum CaretType { CursorCaret, DragCaret };

class RenderBlock : public RenderBox {
public:
    RenderBlock(Node*);
    virtual ~RenderBlock();

    const RenderObjectChildList* children() const { return &m_children; }
    RenderObjectChildList* children() { return &m_children; }

    virtual void destroy();

    // These two functions are overridden for inline-block.
    virtual int lineHeight(bool firstLine, bool isRootLineBox = false) const;
    virtual int baselinePosition(bool firstLine, bool isRootLineBox = false) const;

    RenderLineBoxList* lineBoxes() { return &m_lineBoxes; }
    const RenderLineBoxList* lineBoxes() const { return &m_lineBoxes; }

    InlineFlowBox* firstLineBox() const { return m_lineBoxes.firstLineBox(); }
    InlineFlowBox* lastLineBox() const { return m_lineBoxes.lastLineBox(); }

    void deleteLineBoxTree();

    virtual void addChild(RenderObject* newChild, RenderObject* beforeChild = 0);
    virtual void removeChild(RenderObject*);

    virtual void layoutBlock(bool relayoutChildren);

    void insertPositionedObject(RenderBox*);
    void removePositionedObject(RenderBox*);
    void removePositionedObjects(RenderBlock*);

    void addPercentHeightDescendant(RenderBox*);
    static void removePercentHeightDescendant(RenderBox*);
    HashSet<RenderBox*>* percentHeightDescendants() const;

    RootInlineBox* createAndAppendRootInlineBox();

    bool generatesLineBoxesForInlineChild(RenderObject*, bool isLineEmpty = true, bool previousLineBrokeCleanly = true);

    void markAllDescendantsWithFloatsForLayout(RenderBox* floatToRemove = 0, bool inLayout = true);
    void markPositionedObjectsForLayout();

    bool containsFloats() { return m_floatingObjects && !m_floatingObjects->isEmpty(); }
    bool containsFloat(RenderObject*);

    IntRect floatRect() const;

    int lineWidth(int y, bool firstLine) const;
    
    virtual int lowestPosition(bool includeOverflowInterior = true, bool includeSelf = true) const;
    virtual int rightmostPosition(bool includeOverflowInterior = true, bool includeSelf = true) const;
    virtual int leftmostPosition(bool includeOverflowInterior = true, bool includeSelf = true) const;

    int rightOffset(int y, bool firstLine) const { return rightRelOffset(y, rightOffset(), firstLine); }
    int leftOffset(int y, bool firstLine) const { return leftRelOffset(y, leftOffset(), firstLine); }

    virtual VisiblePosition positionForPoint(const IntPoint&);
    
    // Block flows subclass availableWidth to handle multi column layout (shrinking the width available to children when laying out.)
    virtual int availableWidth() const;
    
    RootInlineBox* firstRootBox() const { return static_cast<RootInlineBox*>(firstLineBox()); }
    RootInlineBox* lastRootBox() const { return static_cast<RootInlineBox*>(lastLineBox()); }

    bool containsNonZeroBidiLevel() const;

    virtual void setSelectionState(SelectionState s);

    GapRects selectionGapRectsForRepaint(RenderBoxModelObject* repaintContainer);
    IntRect fillLeftSelectionGap(RenderObject* selObj, int xPos, int yPos, int height, RenderBlock* rootBlock, 
                                 int blockX, int blockY, int tx, int ty, const PaintInfo*);
    IntRect fillRightSelectionGap(RenderObject* selObj, int xPos, int yPos, int height, RenderBlock* rootBlock,
                                  int blockX, int blockY, int tx, int ty, const PaintInfo*);
    IntRect fillHorizontalSelectionGap(RenderObject* selObj, int xPos, int yPos, int width, int height, const PaintInfo*);

    void getHorizontalSelectionGapInfo(SelectionState, bool& leftGap, bool& rightGap);

    // Helper methods for computing line counts and heights for line counts.
    RootInlineBox* lineAtIndex(int);
    int lineCount();
    int heightForLineCount(int);
    void clearTruncation();

    void adjustRectForColumns(IntRect&) const;
    //Samsung - patch from r54784
    virtual void adjustForColumns(IntSize&, const IntPoint&) const;

    void addContinuationWithOutline(RenderInline*);

    RenderInline* inlineContinuation() const { return m_inlineContinuation; }
    void setInlineContinuation(RenderInline* c) { m_inlineContinuation = c; }

    // This function is a convenience helper for creating an anonymous block that inherits its
    // style from this RenderBlock.
    RenderBlock* createAnonymousBlock(bool isFlexibleBox = false) const;

    static void appendRunsForObject(int start, int end, RenderObject*, InlineBidiResolver&);    
    static bool requiresLineBox(const InlineIterator&, bool isLineEmpty = true, bool previousLineBrokeCleanly = true);

protected:
    void moveChildTo(RenderObject* to, RenderObjectChildList* toChildList, RenderObject* child);
    void moveChildTo(RenderObject* to, RenderObjectChildList* toChildList, RenderObject* beforeChild, RenderObject* child);
    void moveAllChildrenTo(RenderObject* to, RenderObjectChildList* toChildList);
    void moveAllChildrenTo(RenderObject* to, RenderObjectChildList* toChildList, RenderObject* beforeChild);

    int maxTopPosMargin() const { return m_maxMargin ? m_maxMargin->m_topPos : MaxMargin::topPosDefault(this); }
    int maxTopNegMargin() const { return m_maxMargin ? m_maxMargin->m_topNeg : MaxMargin::topNegDefault(this); }
    int maxBottomPosMargin() const { return m_maxMargin ? m_maxMargin->m_bottomPos : MaxMargin::bottomPosDefault(this); }
    int maxBottomNegMargin() const { return m_maxMargin ? m_maxMargin->m_bottomNeg : MaxMargin::bottomNegDefault(this); }
    void setMaxTopMargins(int pos, int neg);
    void setMaxBottomMargins(int pos, int neg);
    
    void initMaxMarginValues()
    {
        if (m_maxMargin) {
            m_maxMargin->m_topPos = MaxMargin::topPosDefault(this);
            m_maxMargin->m_topNeg = MaxMargin::topNegDefault(this);
            m_maxMargin->m_bottomPos = MaxMargin::bottomPosDefault(this);
            m_maxMargin->m_bottomNeg = MaxMargin::bottomNegDefault(this);
        }
    }

    virtual void layout();

    void layoutPositionedObjects(bool relayoutChildren);

    virtual void paint(PaintInfo&, int tx, int ty);
    virtual void paintObject(PaintInfo&, int tx, int ty);

    int rightRelOffset(int y, int fixedOffset, bool applyTextIndent = true, int* heightRemaining = 0) const;
    int leftRelOffset(int y, int fixedOffset, bool applyTextIndent = true, int* heightRemaining = 0) const;

    virtual bool nodeAtPoint(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, HitTestAction);

    virtual void calcPrefWidths();

    virtual int firstLineBoxBaseline() const;
    virtual int lastLineBoxBaseline() const;

    virtual void updateFirstLetter();

    virtual void updateHitTestResult(HitTestResult&, const IntPoint&);

    // Delay update scrollbar until finishDelayRepaint() will be
    // called. This function is used when a flexbox is laying out its
    // descendant. If multiple calls are made to startDelayRepaint(),
    // finishDelayRepaint() will do nothing until finishDelayRepaint()
    // is called the same number of times.
    static void startDelayUpdateScrollInfo();
    static void finishDelayUpdateScrollInfo();

    virtual void styleWillChange(StyleDifference, const RenderStyle* newStyle);
    virtual void styleDidChange(StyleDifference, const RenderStyle* oldStyle);

    virtual bool hasLineIfEmpty() const;
    bool layoutOnlyPositionedObjects();

private:
    virtual RenderObjectChildList* virtualChildren() { return children(); }
    virtual const RenderObjectChildList* virtualChildren() const { return children(); }

    virtual const char* renderName() const;

    virtual bool isRenderBlock() const { return true; }
    virtual bool isBlockFlow() const { return (!isInline() || isReplaced()) && !isTable(); }
    virtual bool isInlineBlockOrInlineTable() const { return isInline() && isReplaced(); }

    void makeChildrenNonInline(RenderObject* insertionPoint = 0);
    virtual void removeLeftoverAnonymousBlock(RenderBlock* child);

    virtual void dirtyLinesFromChangedChild(RenderObject* child) { m_lineBoxes.dirtyLinesFromChangedChild(this, child); }

    virtual bool isSelfCollapsingBlock() const;

    virtual int maxTopMargin(bool positive) const { return positive ? maxTopPosMargin() : maxTopNegMargin(); }
    virtual int maxBottomMargin(bool positive) const { return positive ? maxBottomPosMargin() : maxBottomNegMargin(); }

    virtual void repaintOverhangingFloats(bool paintAllDescendants);

    void layoutBlockChildren(bool relayoutChildren, int& maxFloatBottom);
    void layoutInlineChildren(bool relayoutChildren, int& repaintTop, int& repaintBottom);

    virtual void positionListMarker() { }

    virtual void borderFitAdjust(int& x, int& w) const; // Shrink the box in which the border paints if border-fit is set.

    virtual void updateBeforeAfterContent(PseudoId);

    virtual RootInlineBox* createRootInlineBox(); // Subclassed by SVG and Ruby.

    // Called to lay out the legend for a fieldset.
    virtual RenderObject* layoutLegend(bool /*relayoutChildren*/) { return 0; }

    struct FloatWithRect {
        FloatWithRect(RenderBox* f)
            : object(f)
            , rect(IntRect(f->x() - f->marginLeft(), f->y() - f->marginTop(), f->width() + f->marginLeft() + f->marginRight(), f->height() + f->marginTop() + f->marginBottom()))
            , everHadLayout(f->m_everHadLayout)
        {
        }

        RenderBox* object;
        IntRect rect;
        bool everHadLayout;
    };

    // The following functions' implementations are in RenderBlockLineLayout.cpp.
    void bidiReorderLine(InlineBidiResolver&, const InlineIterator& end, bool previousLineBrokeCleanly);
    RootInlineBox* determineStartPosition(bool& firstLine, bool& fullLayout, bool& previousLineBrokeCleanly,
                                          InlineBidiResolver&, Vector<FloatWithRect>& floats, unsigned& numCleanFloats);
    RootInlineBox* determineEndPosition(RootInlineBox* startBox, InlineIterator& cleanLineStart,
                                        BidiStatus& cleanLineBidiStatus,
                                        int& yPos);
    bool matchedEndLine(const InlineBidiResolver&, const InlineIterator& endLineStart, const BidiStatus& endLineStatus,
                        RootInlineBox*& endLine, int& endYPos, int& repaintBottom, int& repaintTop);

    void skipTrailingWhitespace(InlineIterator&, bool isLineEmpty, bool previousLineBrokeCleanly);
    int skipLeadingWhitespace(InlineBidiResolver&, bool firstLine, bool isLineEmpty, bool previousLineBrokeCleanly);
    void fitBelowFloats(int widthToFit, bool firstLine, int& availableWidth);
    InlineIterator findNextLineBreak(InlineBidiResolver&, bool firstLine, bool& isLineEmpty, bool& previousLineBrokeCleanly, EClear* clear = 0);
    RootInlineBox* constructLine(unsigned runCount, BidiRun* firstRun, BidiRun* lastRun, bool firstLine, bool lastLine, RenderObject* endObject);
    InlineFlowBox* createLineBoxes(RenderObject*, bool firstLine);
    void computeHorizontalPositionsForLine(RootInlineBox*, bool firstLine, BidiRun* firstRun, BidiRun* trailingSpaceRun, bool reachedEnd);
    void computeVerticalPositionsForLine(RootInlineBox*, BidiRun*);
    void deleteEllipsisLineBoxes();
    void checkLinesForTextOverflow();
    void addOverflowFromInlineChildren();
    // End of functions defined in RenderBlockLineLayout.cpp.

    void addOverflowFromBlockChildren();
    void addOverflowFromFloats();

    void paintFloats(PaintInfo&, int tx, int ty, bool preservePhase = false);
    void paintContents(PaintInfo&, int tx, int ty);
    void paintColumnContents(PaintInfo&, int tx, int ty, bool paintFloats = false);
    void paintColumnRules(PaintInfo&, int tx, int ty);
    void paintChildren(PaintInfo&, int tx, int ty);
    void paintEllipsisBoxes(PaintInfo&, int tx, int ty);
    void paintSelection(PaintInfo&, int tx, int ty);
    void paintCaret(PaintInfo&, int tx, int ty, CaretType);

    void insertFloatingObject(RenderBox*);
    void removeFloatingObject(RenderBox*);

    // Called from lineWidth, to position the floats added in the last line.
    // Returns ture if and only if it has positioned any floats.
    bool positionNewFloats();
    void clearFloats();
    int getClearDelta(RenderBox* child, int yPos);

    virtual bool avoidsFloats() const;

    bool hasOverhangingFloats() { return parent() && !hasColumns() && floatBottom() > height(); }
    void addIntrudingFloats(RenderBlock* prev, int xoffset, int yoffset);
    int addOverhangingFloats(RenderBlock* child, int xoffset, int yoffset, bool makeChildPaintOtherFloats);

    int nextFloatBottomBelow(int) const;
    int floatBottom() const;
    inline int leftBottom();
    inline int rightBottom();

    int rightOffset() const;
    int leftOffset() const;
    virtual bool hitTestColumns(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, HitTestAction);
    virtual bool hitTestContents(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, HitTestAction);

    virtual bool isPointInOverflowControl(HitTestResult&, int x, int y, int tx, int ty);

    void calcInlinePrefWidths();
    void calcBlockPrefWidths();

    // Obtains the nearest enclosing block (including this block) that contributes a first-line style to our inline
    // children.
    virtual RenderBlock* firstLineBlock() const;
    bool inRootBlockContext() const;

    virtual IntRect rectWithOutlineForRepaint(RenderBoxModelObject* repaintContainer, int outlineWidth);
    virtual RenderStyle* outlineStyleForRepaint() const;
    
    virtual RenderObject* hoverAncestor() const;
    virtual void updateDragState(bool dragOn);
    virtual void childBecameNonInline(RenderObject* child);

    virtual IntRect selectionRectForRepaint(RenderBoxModelObject* repaintContainer, bool /*clipToVisibleContent*/)
    {
        return selectionGapRectsForRepaint(repaintContainer);
    }
    virtual bool shouldPaintSelectionGaps() const;
    bool isSelectionRoot() const;
    GapRects fillSelectionGaps(RenderBlock* rootBlock, int blockX, int blockY, int tx, int ty,
                               int& lastTop, int& lastLeft, int& lastRight, const PaintInfo* = 0);
    GapRects fillInlineSelectionGaps(RenderBlock* rootBlock, int blockX, int blockY, int tx, int ty,
                                     int& lastTop, int& lastLeft, int& lastRight, const PaintInfo*);
    GapRects fillBlockSelectionGaps(RenderBlock* rootBlock, int blockX, int blockY, int tx, int ty,
                                    int& lastTop, int& lastLeft, int& lastRight, const PaintInfo*);
    IntRect fillVerticalSelectionGap(int lastTop, int lastLeft, int lastRight, int bottomY, RenderBlock* rootBlock,
                                     int blockX, int blockY, const PaintInfo*);
    int leftSelectionOffset(RenderBlock* rootBlock, int y);
    int rightSelectionOffset(RenderBlock* rootBlock, int y);

    virtual void absoluteRects(Vector<IntRect>&, int tx, int ty);
    virtual void absoluteQuads(Vector<FloatQuad>&);

    int desiredColumnWidth() const;
    unsigned desiredColumnCount() const;
#if PLATFORM(ANDROID)
public:
#endif
    Vector<IntRect>* columnRects() const;
#if PLATFORM(ANDROID)
private:
#endif
    void setDesiredColumnCountAndWidth(int count, int width);
#if PLATFORM(ANDROID)
public:
#endif
    int columnGap() const;
#if PLATFORM(ANDROID)
private:
#endif
    
    void paintContinuationOutlines(PaintInfo&, int tx, int ty);

    virtual IntRect localCaretRect(InlineBox*, int caretOffset, int* extraWidthToEndOfLine = 0);

    virtual void addFocusRingRects(Vector<IntRect>&, int tx, int ty);

    void adjustPointToColumnContents(IntPoint&) const;
    void adjustForBorderFit(int x, int& left, int& right) const; // Helper function for borderFitAdjust

    void markLinesDirtyInVerticalRange(int top, int bottom, RootInlineBox* highest = 0);

    void newLine(EClear);

    Position positionForBox(InlineBox*, bool start = true) const;
    Position positionForRenderer(RenderObject*, bool start = true) const;
    VisiblePosition positionForPointWithInlineChildren(const IntPoint&);

    // Adjust tx and ty from painting offsets to the local coords of this renderer
    void offsetForContents(int& tx, int& ty) const;

    void calcColumnWidth();
    int layoutColumns(int endOfContent = -1, int requestedColumnHeight = -1);

    bool expandsToEncloseOverhangingFloats() const;

    void updateScrollInfoAfterLayout();

    struct FloatingObject : Noncopyable {
        enum Type {
            FloatLeft,
            FloatRight
        };

        FloatingObject(Type type)
            : m_renderer(0)
            , m_top(0)
            , m_bottom(0)
            , m_left(0)
            , m_width(0)
            , m_type(type)
            , m_shouldPaint(true)
            , m_isDescendant(false)
        {
        }

        Type type() { return static_cast<Type>(m_type); }

        RenderBox* m_renderer;
        int m_top;
        int m_bottom;
        int m_left;
        int m_width;
        unsigned m_type : 1; // Type (left or right aligned)
        bool m_shouldPaint : 1;
        bool m_isDescendant : 1;
    };

    class MarginInfo {
        // Collapsing flags for whether we can collapse our margins with our children's margins.
        bool m_canCollapseWithChildren : 1;
        bool m_canCollapseTopWithChildren : 1;
        bool m_canCollapseBottomWithChildren : 1;

        // Whether or not we are a quirky container, i.e., do we collapse away top and bottom
        // margins in our container.  Table cells and the body are the common examples. We
        // also have a custom style property for Safari RSS to deal with TypePad blog articles.
        bool m_quirkContainer : 1;

        // This flag tracks whether we are still looking at child margins that can all collapse together at the beginning of a block.  
        // They may or may not collapse with the top margin of the block (|m_canCollapseTopWithChildren| tells us that), but they will
        // always be collapsing with one another.  This variable can remain set to true through multiple iterations 
        // as long as we keep encountering self-collapsing blocks.
        bool m_atTopOfBlock : 1;

        // This flag is set when we know we're examining bottom margins and we know we're at the bottom of the block.
        bool m_atBottomOfBlock : 1;

        // These variables are used to detect quirky margins that we need to collapse away (in table cells
        // and in the body element).
        bool m_topQuirk : 1;
        bool m_bottomQuirk : 1;
        bool m_determinedTopQuirk : 1;

        // These flags track the previous maximal positive and negative margins.
        int m_posMargin;
        int m_negMargin;

    public:
        MarginInfo(RenderBlock* b, int top, int bottom);

        void setAtTopOfBlock(bool b) { m_atTopOfBlock = b; }
        void setAtBottomOfBlock(bool b) { m_atBottomOfBlock = b; }
        void clearMargin() { m_posMargin = m_negMargin = 0; }
        void setTopQuirk(bool b) { m_topQuirk = b; }
        void setBottomQuirk(bool b) { m_bottomQuirk = b; }
        void setDeterminedTopQuirk(bool b) { m_determinedTopQuirk = b; }
        void setPosMargin(int p) { m_posMargin = p; }
        void setNegMargin(int n) { m_negMargin = n; }
        void setPosMarginIfLarger(int p) { if (p > m_posMargin) m_posMargin = p; }
        void setNegMarginIfLarger(int n) { if (n > m_negMargin) m_negMargin = n; }

        void setMargin(int p, int n) { m_posMargin = p; m_negMargin = n; }

        bool atTopOfBlock() const { return m_atTopOfBlock; }
        bool canCollapseWithTop() const { return m_atTopOfBlock && m_canCollapseTopWithChildren; }
        bool canCollapseWithBottom() const { return m_atBottomOfBlock && m_canCollapseBottomWithChildren; }
        bool canCollapseTopWithChildren() const { return m_canCollapseTopWithChildren; }
        bool canCollapseBottomWithChildren() const { return m_canCollapseBottomWithChildren; }
        bool quirkContainer() const { return m_quirkContainer; }
        bool determinedTopQuirk() const { return m_determinedTopQuirk; }
        bool topQuirk() const { return m_topQuirk; }
        bool bottomQuirk() const { return m_bottomQuirk; }
        int posMargin() const { return m_posMargin; }
        int negMargin() const { return m_negMargin; }
        int margin() const { return m_posMargin - m_negMargin; }
    };

    void layoutBlockChild(RenderBox* child, MarginInfo&, int& previousFloatBottom, int& maxFloatBottom);
    void adjustPositionedBlock(RenderBox* child, const MarginInfo&);
    void adjustFloatingBlock(const MarginInfo&);
    bool handleSpecialChild(RenderBox* child, const MarginInfo&);
    bool handleFloatingChild(RenderBox* child, const MarginInfo&);
    bool handlePositionedChild(RenderBox* child, const MarginInfo&);
    bool handleRunInChild(RenderBox* child);
    int collapseMargins(RenderBox* child, MarginInfo&);
    int clearFloatsIfNeeded(RenderBox* child, MarginInfo&, int oldTopPosMargin, int oldTopNegMargin, int yPos);
    int estimateVerticalPosition(RenderBox* child, const MarginInfo&);
    void determineHorizontalPosition(RenderBox* child);
    void handleBottomOfBlock(int top, int bottom, MarginInfo&);
    void setCollapsedBottomMargin(const MarginInfo&);
    // End helper functions and structs used by layoutBlockChildren.

    typedef ListHashSet<RenderBox*>::const_iterator Iterator;
    DeprecatedPtrList<FloatingObject>* m_floatingObjects;
    ListHashSet<RenderBox*>* m_positionedObjects;

    // An inline can be split with blocks occurring in between the inline content.
    // When this occurs we need a pointer to our next object.  We can basically be
    // split into a sequence of inlines and blocks.  The continuation will either be
    // an anonymous block (that houses other blocks) or it will be an inline flow.
    RenderInline* m_inlineContinuation;

    // Allocated only when some of these fields have non-default values
    struct MaxMargin : Noncopyable {
        MaxMargin(const RenderBlock* o) 
            : m_topPos(topPosDefault(o))
            , m_topNeg(topNegDefault(o))
            , m_bottomPos(bottomPosDefault(o))
            , m_bottomNeg(bottomNegDefault(o))
        { 
        }

        static int topPosDefault(const RenderBlock* o) { return o->marginTop() > 0 ? o->marginTop() : 0; }
        static int topNegDefault(const RenderBlock* o) { return o->marginTop() < 0 ? -o->marginTop() : 0; }
        static int bottomPosDefault(const RenderBlock* o) { return o->marginBottom() > 0 ? o->marginBottom() : 0; }
        static int bottomNegDefault(const RenderBlock* o) { return o->marginBottom() < 0 ? -o->marginBottom() : 0; }

        int m_topPos;
        int m_topNeg;
        int m_bottomPos;
        int m_bottomNeg;
     };

    MaxMargin* m_maxMargin;

    RenderObjectChildList m_children;
    RenderLineBoxList m_lineBoxes;   // All of the root line boxes created for this block flow.  For example, <div>Hello<br>world.</div> will have two total lines for the <div>.

    mutable int m_lineHeight;
    
    // RenderRubyBase objects need to be able to split and merge, moving their children around
    // (calling moveChildTo, moveAllChildrenTo, and makeChildrenNonInline).
    friend class RenderRubyBase;
};

inline RenderBlock* toRenderBlock(RenderObject* object)
{ 
    ASSERT(!object || object->isRenderBlock());
    return static_cast<RenderBlock*>(object);
}

inline const RenderBlock* toRenderBlock(const RenderObject* object)
{ 
    ASSERT(!object || object->isRenderBlock());
    return static_cast<const RenderBlock*>(object);
}

// This will catch anyone doing an unnecessary cast.
void toRenderBlock(const RenderBlock*);

} // namespace WebCore

#endif // RenderBlock_h
