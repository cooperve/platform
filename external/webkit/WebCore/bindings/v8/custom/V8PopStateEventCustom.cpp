/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "V8PopStateEvent.h"

#include "PopStateEvent.h"
#include "SerializedScriptValue.h"
#include "V8Proxy.h"

namespace WebCore {

v8::Handle<v8::Value> V8PopStateEvent::initPopStateEventCallback(const v8::Arguments& args)
{
    INC_STATS("DOM.PopStateEvent.initPopStateEvent");

    String typeArg = v8ValueToWebCoreString(args[0]);
    bool canBubbleArg = args[1]->BooleanValue();
    bool cancelableArg = args[2]->BooleanValue();
    RefPtr<SerializedScriptValue> stateArg = SerializedScriptValue::create(args[3]);

    PopStateEvent* event = V8PopStateEvent::toNative(args.Holder());
    event->initPopStateEvent(typeArg, canBubbleArg, cancelableArg, stateArg.release());

    return v8::Undefined();
}

v8::Handle<v8::Value> V8PopStateEvent::stateAccessorGetter(v8::Local<v8::String> name, const v8::AccessorInfo& info)
{
    INC_STATS("DOM.PopStateEvent.state");

    PopStateEvent* event = V8PopStateEvent::toNative(info.Holder());
    SerializedScriptValue* state = event->state();
    if (!state)
        return v8::Null();

    return state->deserialize();
}

} // namespace WebCore
