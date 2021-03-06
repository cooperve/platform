/*
 * Copyright (c) 2008, 2009, Google Inc. All rights reserved.
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

#ifndef ChromiumDataObject_h
#define ChromiumDataObject_h

#include "KURL.h"
#include "PlatformString.h"
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

    // A data object for holding data that would be in a clipboard or moved
    // during a drag-n-drop operation.  This is the data that WebCore is aware
    // of and is not specific to a platform.
    class ChromiumDataObject : public RefCounted<ChromiumDataObject> {
    public:
        static PassRefPtr<ChromiumDataObject> create()
        {
            return adoptRef(new ChromiumDataObject);
        }

        PassRefPtr<ChromiumDataObject> copy() const
        {
            return adoptRef(new ChromiumDataObject(*this));
        }

        void clear();
        bool hasData() const;

        KURL url;
        String urlTitle;

        KURL downloadURL;
        String downloadMetadata;

        String fileExtension;
        Vector<String> filenames;

        String plainText;

        String textHtml;
        KURL htmlBaseUrl;

        String fileContentFilename;
        RefPtr<SharedBuffer> fileContent;

    private:
        ChromiumDataObject() {}
        ChromiumDataObject(const ChromiumDataObject&);
    };

} // namespace WebCore

#endif
