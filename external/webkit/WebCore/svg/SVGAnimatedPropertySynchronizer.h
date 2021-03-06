/*
    Copyright (C) Research In Motion Limited 2010. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SVGAnimatedPropertySynchronizer_h
#define SVGAnimatedPropertySynchronizer_h

#include "SVGAnimatedPropertyTraits.h"

#if ENABLE(SVG)
namespace WebCore {

class SVGElement;

// GetOwnerElementForType implementation
template<typename OwnerType, bool isDerivedFromSVGElement>
struct GetOwnerElementForType;

template<typename OwnerType>
struct GetOwnerElementForType<OwnerType, true> : public Noncopyable {
    static SVGElement* ownerElement(OwnerType* type)
    {
        return type;
    }
};

template<typename OwnerType>
struct GetOwnerElementForType<OwnerType, false> : public Noncopyable {    
    static SVGElement* ownerElement(OwnerType* type)
    {
        SVGElement* context = type->contextElement();
        ASSERT(context);
        return context;
    }
};

// IsDerivedFromSVGElement implementation
template<typename OwnerType>
struct IsDerivedFromSVGElement : public Noncopyable {
    static const bool value = true;
};

class SVGViewSpec;
template<>
struct IsDerivedFromSVGElement<SVGViewSpec> : public Noncopyable {
    static const bool value = false;
};

// Helper template used for synchronizing SVG <-> XML properties
template<bool isDerivedFromSVGElement>
struct SVGAnimatedPropertySynchronizer {
    static void synchronize(SVGElement*, const QualifiedName&, const AtomicString&);
};

template<>
struct SVGAnimatedPropertySynchronizer<true> {
    static void synchronize(SVGElement* ownerElement, const QualifiedName& attrName, const AtomicString& value)
    {
        NamedNodeMap* namedAttrMap = ownerElement->attributes(false); 
        Attribute* old = namedAttrMap->getAttributeItem(attrName);
        if (old && value.isNull()) 
            namedAttrMap->removeAttribute(old->name()); 
        else if (!old && !value.isNull()) 
            namedAttrMap->addAttribute(ownerElement->createAttribute(attrName, value));
        else if (old && !value.isNull()) 
            old->setValue(value); 
    }
};

template<>
struct SVGAnimatedPropertySynchronizer<false> {
    static void synchronize(SVGElement*, const QualifiedName&, const AtomicString&)
    {
        // no-op, for types not inheriting from Element, thus nothing to synchronize
    }
};

};

#endif
#endif
