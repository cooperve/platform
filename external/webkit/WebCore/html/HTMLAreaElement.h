/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2008, 2009 Apple Inc. All rights reserved.
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
 *
 */

#ifndef HTMLAreaElement_h
#define HTMLAreaElement_h

#include "HTMLAnchorElement.h"
#include "IntSize.h"
#include <wtf/OwnArrayPtr.h>

namespace WebCore {

class HitTestResult;
class HTMLImageElement;
class Path;

class HTMLAreaElement : public HTMLAnchorElement {
public:
    static PassRefPtr<HTMLAreaElement> create(const QualifiedName&, Document*);

    bool isDefault() const { return m_shape == Default; }

    bool mapMouseEvent(int x, int y, const IntSize&, HitTestResult&);

    IntRect getRect(RenderObject*) const;
    Path getPath(RenderObject*) const;
    
    // Convenience method to get the parent map's image.
    HTMLImageElement* imageElement() const;
    
    KURL href() const;

    bool noHref() const;
    void setNoHref(bool);

    //SAMSUNG CHANGE >>
    enum Shape { Default, Poly, Rect, Circle, Unknown };
	Shape shape() const { return m_shape; }
	IntRect rect() const;
	//SAMSUNG CHANGE <<

private:
    HTMLAreaElement(const QualifiedName&, Document*);

    virtual HTMLTagStatus endTagRequirement() const { return TagStatusForbidden; }
    virtual int tagPriority() const { return 0; }
    virtual void parseMappedAttribute(MappedAttribute*);
    virtual bool supportsFocus() const;
    virtual String target() const;
    virtual bool isKeyboardFocusable(KeyboardEvent*) const;
    virtual bool isFocusable() const;
    virtual void updateFocusAppearance(bool /*restorePreviousSelection*/);
    virtual void dispatchBlurEvent();
    
    //enum Shape { Default, Poly, Rect, Circle, Unknown }; //SAMSUNG CHANGE
    Path getRegion(const IntSize&) const;

    OwnPtr<Path> m_region;
    OwnArrayPtr<Length> m_coords;
    int m_coordsLen;
    IntSize m_lastSize;
    Shape m_shape;
};

} //namespace

#endif
