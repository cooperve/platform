/*
 * Copyright (C) 2007 Apple Computer, Inc.
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

#ifndef FontCustomPlatformData_h
#define FontCustomPlatformData_h

#include "FontRenderingMode.h"
#include <CoreFoundation/CFBase.h>
#include <wtf/Noncopyable.h>

typedef struct CGFont* CGFontRef;
typedef UInt32 ATSFontContainerRef;
typedef UInt32 ATSFontRef;

namespace WebCore {

class FontPlatformData;
class SharedBuffer;

struct FontCustomPlatformData : Noncopyable {
    FontCustomPlatformData(ATSFontContainerRef container, ATSFontRef atsFont, CGFontRef cgFont)
    : m_atsContainer(container), m_atsFont(atsFont), m_cgFont(cgFont)
    {}
    ~FontCustomPlatformData();

    FontPlatformData fontPlatformData(int size, bool bold, bool italic, FontRenderingMode = NormalRenderingMode);

    ATSFontContainerRef m_atsContainer;
    ATSFontRef m_atsFont;
    CGFontRef m_cgFont;
};

FontCustomPlatformData* createFontCustomPlatformData(SharedBuffer* buffer);

}

#endif
