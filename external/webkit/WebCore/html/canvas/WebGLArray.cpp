/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

#if ENABLE(3D_CANVAS)

#include "WebGLArray.h"
#include "WebGLArrayBuffer.h"

namespace WebCore {

WebGLArray::WebGLArray(PassRefPtr<WebGLArrayBuffer> buffer,
                       unsigned byteOffset)
        : m_byteOffset(byteOffset)
        , m_buffer(buffer)
{
    m_baseAddress = m_buffer ? (static_cast<char*>(m_buffer->data()) + m_byteOffset) : 0;
}

WebGLArray::~WebGLArray()
{
}

void WebGLArray::setImpl(WebGLArray* array, unsigned byteOffset, ExceptionCode& ec)
{
    if (byteOffset + array->byteLength() > byteLength()) {
        ec = INDEX_SIZE_ERR;
        return;
    }

    char* base = static_cast<char*>(baseAddress());
    memcpy(base + byteOffset, array->baseAddress(), array->byteLength());
}

}

#endif // ENABLE(3D_CANVAS)
