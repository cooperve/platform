/*
 * Copyright (C) 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
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

#ifndef RenderMedia_h
#define RenderMedia_h

#if ENABLE(VIDEO)

#include "RenderImage.h"
#include "Timer.h"

namespace WebCore {
    
class HTMLInputElement;
class HTMLMediaElement;
class MediaControlMuteButtonElement;
class MediaControlPlayButtonElement;
class MediaControlSeekButtonElement;
class MediaControlRewindButtonElement;
class MediaControlReturnToRealtimeButtonElement;
class MediaControlToggleClosedCaptionsButtonElement;
class MediaControlTimelineElement;
class MediaControlVolumeSliderElement;
class MediaControlFullscreenButtonElement;
class MediaControlTimeDisplayElement;
class MediaControlStatusDisplayElement;
class MediaControlTimelineContainerElement;
class MediaControlVolumeSliderContainerElement;
class MediaControlElement;
class MediaPlayer;

class RenderMedia : public RenderImage {
public:
    RenderMedia(HTMLMediaElement*);
    RenderMedia(HTMLMediaElement*, const IntSize& intrinsicSize);
    virtual ~RenderMedia();

    const RenderObjectChildList* children() const { return &m_children; }
    RenderObjectChildList* children() { return &m_children; }

    HTMLMediaElement* mediaElement() const;
    MediaPlayer* player() const;

    bool shouldShowTimeDisplayControls() const;

    void updateFromElement();
    void updatePlayer();
    void updateControls();
    void updateTimeDisplay();
    
    void forwardEvent(Event*);

protected:
    virtual void layout();

private:
    virtual RenderObjectChildList* virtualChildren() { return children(); }
    virtual const RenderObjectChildList* virtualChildren() const { return children(); }

    virtual void destroy();
    
    virtual const char* renderName() const { return "RenderMedia"; }
    virtual bool isMedia() const { return true; }
    virtual bool isImage() const { return false; }

    virtual int lowestPosition(bool includeOverflowInterior = true, bool includeSelf = true) const;
    virtual int rightmostPosition(bool includeOverflowInterior = true, bool includeSelf = true) const;
    virtual int leftmostPosition(bool includeOverflowInterior = true, bool includeSelf = true) const;

    void createControlsShadowRoot();
    void destroyControlsShadowRoot();
    void createPanel();
    void createMuteButton();
    void createPlayButton();
    void createSeekBackButton();
    void createSeekForwardButton();
    void createRewindButton();
    void createReturnToRealtimeButton();
    void createToggleClosedCaptionsButton();
    void createStatusDisplay();
    void createTimelineContainer();
    void createTimeline();
    void createVolumeSliderContainer();
    void createVolumeSlider();
    void createCurrentTimeDisplay();
    void createTimeRemainingDisplay();
    void createFullscreenButton();
    
    void timeUpdateTimerFired(Timer<RenderMedia>*);
    
    void updateControlVisibility();
    void changeOpacity(HTMLElement*, float opacity);
    void opacityAnimationTimerFired(Timer<RenderMedia>*);

    void updateVolumeSliderContainer(bool visible);

    virtual void styleDidChange(StyleDifference, const RenderStyle* oldStyle);

    RefPtr<HTMLElement> m_controlsShadowRoot;
    RefPtr<MediaControlElement> m_panel;
    RefPtr<MediaControlMuteButtonElement> m_muteButton;
    RefPtr<MediaControlPlayButtonElement> m_playButton;
    RefPtr<MediaControlSeekButtonElement> m_seekBackButton;
    RefPtr<MediaControlSeekButtonElement> m_seekForwardButton;
    RefPtr<MediaControlRewindButtonElement> m_rewindButton;
    RefPtr<MediaControlReturnToRealtimeButtonElement> m_returnToRealtimeButton;
    RefPtr<MediaControlToggleClosedCaptionsButtonElement> m_toggleClosedCaptionsButton;
    RefPtr<MediaControlTimelineElement> m_timeline;
    RefPtr<MediaControlVolumeSliderElement> m_volumeSlider;
    RefPtr<MediaControlFullscreenButtonElement> m_fullscreenButton;
    RefPtr<MediaControlTimelineContainerElement> m_timelineContainer;
    RefPtr<MediaControlVolumeSliderContainerElement> m_volumeSliderContainer;
    RefPtr<MediaControlTimeDisplayElement> m_currentTimeDisplay;
    RefPtr<MediaControlTimeDisplayElement> m_timeRemainingDisplay;
    RefPtr<MediaControlStatusDisplayElement> m_statusDisplay;
    RenderObjectChildList m_children;
    Node* m_lastUnderNode;
    Node* m_nodeUnderMouse;
    
    Timer<RenderMedia> m_timeUpdateTimer;
    Timer<RenderMedia> m_opacityAnimationTimer;
    bool m_mouseOver;
    double m_opacityAnimationStartTime;
    double m_opacityAnimationDuration;
    float m_opacityAnimationFrom;
    float m_opacityAnimationTo;
};

inline RenderMedia* toRenderMedia(RenderObject* object)
{
    ASSERT(!object || object->isMedia());
    return static_cast<RenderMedia*>(object);
}

// This will catch anyone doing an unnecessary cast.
void toRenderMedia(const RenderMedia*);

} // namespace WebCore

#endif
#endif // RenderMedia_h
