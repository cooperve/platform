/*
 * Copyright (C) 2007, 2008, 2009 Apple Inc. All rights reserved.
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

#include "config.h"
#include "JSStyleSheet.h"

#include "CSSStyleSheet.h"
#include "Node.h"
#include "JSCSSStyleSheet.h"
#include "JSNode.h"

using namespace JSC;

namespace WebCore {

JSValue toJS(ExecState* exec, JSDOMGlobalObject* globalObject, StyleSheet* styleSheet)
{
    if (!styleSheet)
        return jsNull();

    DOMObject* wrapper = getCachedDOMObjectWrapper(exec, styleSheet);
    if (wrapper)
        return wrapper;

    if (styleSheet->isCSSStyleSheet())
        wrapper = CREATE_DOM_OBJECT_WRAPPER(exec, globalObject, CSSStyleSheet, styleSheet);
    else
        wrapper = CREATE_DOM_OBJECT_WRAPPER(exec, globalObject, StyleSheet, styleSheet);

    return wrapper;
}

void JSStyleSheet::markChildren(MarkStack& markStack)
{
    Base::markChildren(markStack);

    StyleSheet* sheet = impl();
    JSGlobalData& globalData = *Heap::heap(this)->globalData();

    unsigned length = sheet->length();
    for (unsigned i = 0; i < length; ++i)
        markDOMObjectWrapper(markStack, globalData, sheet->item(i));

    // This prevents us from having a style sheet with a dangling ownerNode pointer.
    // A better solution would be to handle this on the DOM side -- if the style sheet
    // is kept around, then we want the node to stay around too. One possibility would
    // be to make ref/deref on the style sheet ref/deref the node instead, but there's
    // a lot of disentangling of the CSS DOM objects that would need to happen first.
    if (Node* ownerNode = sheet->ownerNode())
        markDOMNodeWrapper(markStack, ownerNode->document(), ownerNode);
}

} // namespace WebCore
