/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
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
#include "WebSecurityPolicy.h"

#include "FrameLoader.h"
#include "SecurityOrigin.h"

#include "WebString.h"
#include "WebURL.h"

using namespace WebCore;

namespace WebKit {

void WebSecurityPolicy::registerURLSchemeAsLocal(const WebString& scheme)
{
    SecurityOrigin::registerURLSchemeAsLocal(scheme);
}

void WebSecurityPolicy::registerURLSchemeAsNoAccess(const WebString& scheme)
{
    SecurityOrigin::registerURLSchemeAsNoAccess(scheme);
}

void WebSecurityPolicy::whiteListAccessFromOrigin(const WebURL& sourceOrigin,
    const WebString& destinationProtocol,
    const WebString& destinationHost,
    bool allowDestinationSubdomains)
{
    SecurityOrigin::whiteListAccessFromOrigin(
        *SecurityOrigin::create(sourceOrigin), destinationProtocol,
        destinationHost, allowDestinationSubdomains);
}

void WebSecurityPolicy::resetOriginAccessWhiteLists()
{
    SecurityOrigin::resetOriginAccessWhiteLists();
}

bool WebSecurityPolicy::shouldHideReferrer(const WebURL& url, const WebString& referrer)
{
    return SecurityOrigin::shouldHideReferrer(url, referrer);
}

} // namespace WebKit
