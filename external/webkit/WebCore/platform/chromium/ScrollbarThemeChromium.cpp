/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 * Copyright (C) 2008, 2009 Google Inc.
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
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ScrollbarThemeChromium.h"

#include "ChromiumBridge.h"
#include "PlatformMouseEvent.h"
#include "Scrollbar.h"
#include "ScrollbarClient.h"
#include "ScrollbarThemeComposite.h"

// -----------------------------------------------------------------------------
// This file contains scrollbar theme code that is cross platform. Additional
// members of ScrollbarThemeChromium can be found in the platform specific files
// -----------------------------------------------------------------------------

namespace WebCore {

bool ScrollbarThemeChromium::hasThumb(Scrollbar* scrollbar)
{
    // This method is just called as a paint-time optimization to see if
    // painting the thumb can be skipped.  We don't have to be exact here.
    return thumbLength(scrollbar) > 0;
}

IntRect ScrollbarThemeChromium::backButtonRect(Scrollbar* scrollbar, ScrollbarPart part, bool)
{
    // Windows and Linux just have single arrows.
    if (part == BackButtonEndPart)
        return IntRect();

    IntSize size = buttonSize(scrollbar);
    return IntRect(scrollbar->x(), scrollbar->y(), size.width(), size.height());
}

IntRect ScrollbarThemeChromium::forwardButtonRect(Scrollbar* scrollbar, ScrollbarPart part, bool)
{
    // Windows and Linux just have single arrows.
    if (part == ForwardButtonStartPart)
        return IntRect();

    IntSize size = buttonSize(scrollbar);
    int x, y;
    if (scrollbar->orientation() == HorizontalScrollbar) {
        x = scrollbar->x() + scrollbar->width() - size.width();
        y = scrollbar->y();
    } else {
        x = scrollbar->x();
        y = scrollbar->y() + scrollbar->height() - size.height();
    }
    return IntRect(x, y, size.width(), size.height());
}

void ScrollbarThemeChromium::paintTrackBackground(GraphicsContext* context, Scrollbar* scrollbar, const IntRect& rect)
{
    // Just assume a forward track part.  We only paint the track as a single piece when there is no thumb.
    if (!hasThumb(scrollbar))
        paintTrackPiece(context, scrollbar, rect, ForwardTrackPart);
}

void ScrollbarThemeChromium::paintTickmarks(GraphicsContext* context, Scrollbar* scrollbar, const IntRect& rect)
{
    if (scrollbar->orientation() != VerticalScrollbar)
        return;

    if (rect.height() <= 0 || rect.width() <= 0)
        return;  // nothing to draw on.

    // Get the tickmarks for the frameview.
    Vector<IntRect> tickmarks;
    scrollbar->client()->getTickmarks(tickmarks);
    if (!tickmarks.size())
        return;

    // Get the image for the tickmarks.
    static RefPtr<Image> dash = Image::loadPlatformResource("tickmarkDash");
    if (dash->isNull()) {
        ASSERT_NOT_REACHED();
        return;
    }

    context->save();

    for (Vector<IntRect>::const_iterator i = tickmarks.begin(); i != tickmarks.end(); ++i) {
        // Calculate how far down (in %) the tick-mark should appear.
        const float percent = static_cast<float>(i->y()) / scrollbar->totalSize();

        // Calculate how far down (in pixels) the tick-mark should appear.
        const int yPos = rect.topLeft().y() + (rect.height() * percent);

        IntPoint tick(scrollbar->x(), yPos);
        context->drawImage(dash.get(), DeviceColorSpace, tick);
    }

    context->restore();
}

void ScrollbarThemeChromium::paintScrollCorner(ScrollView* view, GraphicsContext* context, const IntRect& cornerRect)
{
    // ScrollbarThemeComposite::paintScrollCorner incorrectly assumes that the
    // ScrollView is a FrameView (see FramelessScrollView), so we cannot let
    // that code run.  For FrameView's this is correct since we don't do custom
    // scrollbar corner rendering, which ScrollbarThemeComposite supports.
    ScrollbarTheme::paintScrollCorner(view, context, cornerRect);
}

} // namespace WebCore
