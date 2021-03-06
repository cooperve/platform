/*
    Copyright (C) 2008 Alex Mathews <possessedpenguinbob@gmail.com>
                  2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>
                  2005 Eric Seidel <eric@webkit.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SVGLightSource_h
#define SVGLightSource_h

#if ENABLE(SVG) && ENABLE(FILTERS)
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

    enum LightType {
        LS_DISTANT,
        LS_POINT,
        LS_SPOT
    };

    class TextStream;

    class LightSource : public RefCounted<LightSource> {
    public:
        LightSource(LightType type)
            : m_type(type)
        { }

        virtual ~LightSource() { }

        LightType type() const { return m_type; }
        virtual TextStream& externalRepresentation(TextStream&) const = 0;

    private:
        LightType m_type;
    };

} // namespace WebCore

#endif // ENABLE(SVG) && ENABLE(FILTERS)

#endif // SVGLightSource_h
