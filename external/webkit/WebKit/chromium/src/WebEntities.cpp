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
#include "WebEntities.h"

#include <string.h>

#include "PlatformString.h"
#include "StringBuilder.h"
#include <wtf/HashMap.h>

#include "WebString.h"

using namespace WebCore;

namespace {
// Note that this file is also included by HTMLTokenizer.cpp so we are getting
// two copies of the data in memory.  We can fix this by changing the script
// that generated the array to create a static const that is its length, but
// this is low priority since the data is less than 4K. We use anonymous
// namespace to prevent name collisions.
#include "HTMLEntityNames.c" // NOLINT
}

namespace WebKit {

void populateMap(WTF::HashMap<int, WebCore::String>& map,
                 const Entity* entities,
                 size_t entitiesCount,
                 bool standardHTML)
{
    ASSERT(map.isEmpty());
    const Entity* entity = &entities[0];
    for (size_t i = 0; i < entitiesCount; i++, entity++) {
        int code = entity->code;
        String name = entity->name;
        // For consistency, use the lowe case for entities that have both.
        if (map.contains(code) && map.get(code) == name.lower())
            continue;
        // Don't register &percnt;, &nsup; and &supl;.
        if (standardHTML && (code == '%' || code == 0x2285 || code == 0x00b9))
            continue;
        map.set(code, name);
    }
    if (standardHTML)
        map.set(static_cast<int>(0x0027), String("#39"));
}

static const Entity xmlBuiltInEntityCodes[] = {
    { "lt", 0x003c },
    { "gt", 0x003e },
    { "amp", 0x0026 },
    { "apos", 0x0027 },
    { "quot", 0x0022 }
};

WebEntities::WebEntities(bool xmlEntities)
{
    if (xmlEntities)
        populateMap(m_entitiesMap,
                    xmlBuiltInEntityCodes,
                    sizeof(xmlBuiltInEntityCodes) / sizeof(Entity),
                    false);
    else
        populateMap(m_entitiesMap,
                    wordlist,
                    sizeof(wordlist) / sizeof(Entity),
                    true);
}

String WebEntities::entityNameByCode(int code) const
{
    if (m_entitiesMap.contains(code))
        return m_entitiesMap.get(code);
    return "";
}

String WebEntities::convertEntitiesInString(const String& value) const
{
    unsigned len = value.length();
    const UChar* startPos = value.characters();
    const UChar* curPos = startPos;

    // FIXME: Optimize - create StringBuilder only if value has any entities.
    StringBuilder result;
    while (len--) {
        if (m_entitiesMap.contains(*curPos)) {
            // Append content before entity code.
            if (curPos > startPos)
                result.append(String(startPos, curPos - startPos));
            result.append("&");
            result.append(m_entitiesMap.get(*curPos));
            result.append(";");
            startPos = ++curPos;
        } else
            curPos++;
    }
    // Append the remaining content.
    if (curPos > startPos)
        result.append(String(startPos, curPos - startPos));

    return result.toString();
}

} // namespace WebKit
