/*
 * Copyright 2009, The Android Open Source Project
 * Copyright (C) 2003, 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ResourceRequest_h
#define ResourceRequest_h

#include "CachedResource.h"
#include "ResourceRequestBase.h"

namespace WebCore {

class ResourceRequest : public ResourceRequestBase {
public:
    ResourceRequest(const String& url)
        : ResourceRequestBase(KURL(ParsedURLString, url), UseProtocolCachePolicy), m_userGesture(true) { }

    ResourceRequest(const KURL& url) : ResourceRequestBase(url, UseProtocolCachePolicy) , m_userGesture(true) { }

    ResourceRequest(const KURL& url, const String& referrer, ResourceRequestCachePolicy policy = UseProtocolCachePolicy)
        : ResourceRequestBase(url, policy) , m_userGesture(true)
    {
        setHTTPReferrer(referrer);
    }

    ResourceRequest() : ResourceRequestBase(KURL(), UseProtocolCachePolicy), m_userGesture(true) { }

    void doUpdatePlatformRequest() { }
    void doUpdateResourceRequest() { }
    void setUserGesture(bool userGesture) { m_userGesture = userGesture; }
    bool getUserGesture() const { return m_userGesture; }

private:
    friend class ResourceRequestBase;
    bool m_userGesture;
};

} // namespace WebCore

#endif // ResourceRequest_h
