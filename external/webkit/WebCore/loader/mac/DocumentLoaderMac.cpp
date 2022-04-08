/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "DocumentLoader.h"
#include "MainResourceLoader.h"
#include "ResourceHandle.h"
#include "ResourceLoader.h"
#include <wtf/UnusedParam.h>

namespace WebCore {

#ifndef BUILDING_ON_TIGER
static void scheduleAll(const ResourceLoaderSet& loaders, SchedulePair* pair)
{
    const ResourceLoaderSet copy = loaders;
    ResourceLoaderSet::const_iterator end = copy.end();
    for (ResourceLoaderSet::const_iterator it = copy.begin(); it != end; ++it)
        if (ResourceHandle* handle = (*it)->handle())
            handle->schedule(pair);
}

static void unscheduleAll(const ResourceLoaderSet& loaders, SchedulePair* pair)
{
    const ResourceLoaderSet copy = loaders;
    ResourceLoaderSet::const_iterator end = copy.end();
    for (ResourceLoaderSet::const_iterator it = copy.begin(); it != end; ++it)
        if (ResourceHandle* handle = (*it)->handle())
            handle->unschedule(pair);
}
#endif

void DocumentLoader::schedule(SchedulePair* pair)
{
#ifndef BUILDING_ON_TIGER
    if (m_mainResourceLoader && m_mainResourceLoader->handle())
        m_mainResourceLoader->handle()->schedule(pair);
    scheduleAll(m_subresourceLoaders, pair);
    scheduleAll(m_plugInStreamLoaders, pair);
    scheduleAll(m_multipartSubresourceLoaders, pair);
#else
    UNUSED_PARAM(pair);
#endif
}

void DocumentLoader::unschedule(SchedulePair* pair)
{
#ifndef BUILDING_ON_TIGER
    if (m_mainResourceLoader && m_mainResourceLoader->handle())
        m_mainResourceLoader->handle()->unschedule(pair);
    unscheduleAll(m_subresourceLoaders, pair);
    unscheduleAll(m_plugInStreamLoaders, pair);
    unscheduleAll(m_multipartSubresourceLoaders, pair);
#else
    UNUSED_PARAM(pair);
#endif
}

} // namespace
