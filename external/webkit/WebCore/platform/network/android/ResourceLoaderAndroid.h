/*
 * Copyright 2009, The Android Open Source Project
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

#ifndef ResourceLoaderAndroid_h
#define ResourceLoaderAndroid_h

#include <ResourceRequest.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class FrameLoaderClient;
class ResourceHandle;

class ResourceLoaderAndroid : public RefCounted<ResourceLoaderAndroid> {
public:
    static PassRefPtr<ResourceLoaderAndroid> start(ResourceHandle*, const ResourceRequest&, FrameLoaderClient*, bool isMainResource, bool isSync);
    virtual ~ResourceLoaderAndroid() { }

    virtual void cancel() = 0;
    virtual void downloadFile() = 0;
    // ANDROID TODO: This needs to be upstreamed.
    virtual void pauseLoad(bool) = 0;
    // END ANDROID TODO

    // Call to java to find out if this URL is in the cache
    static bool willLoadFromCache(const WebCore::KURL&, int64_t identifier);
protected:
    ResourceLoaderAndroid() { }
};

}
#endif // ResourceLoaderAndroid
