/*
 * Copyright 2007, The Android Open Source Project
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

#ifndef CachedNodeType_H
#define CachedNodeType_H

namespace android {

enum CachedNodeType {
    NORMAL_CACHEDNODETYPE,
    ADDRESS_CACHEDNODETYPE,
    EMAIL_CACHEDNODETYPE,
    PHONE_CACHEDNODETYPE,
    ANCHOR_CACHEDNODETYPE,
    AREA_CACHEDNODETYPE,
    FRAME_CACHEDNODETYPE,
    PLUGIN_CACHEDNODETYPE,
    TEXT_INPUT_CACHEDNODETYPE,
    //SAMSUNG CHANGE >>
    TEXT_INPUT_READONLY_CACHEDNODETYPE,
    SELECT_CACHEDNODETYPE
    //SAMSUNG CHANGE <<
};

enum CachedNodeBits {
    NORMAL_CACHEDNODE_BITS = 0,
    ADDRESS_CACHEDNODE_BIT = 1 << (ADDRESS_CACHEDNODETYPE - 1),
    EMAIL_CACHEDNODE_BIT = 1 << (EMAIL_CACHEDNODETYPE - 1),
    PHONE_CACHEDNODE_BIT = 1 << (PHONE_CACHEDNODETYPE - 1),
    ALL_CACHEDNODE_BITS = ADDRESS_CACHEDNODE_BIT | EMAIL_CACHEDNODE_BIT
        | PHONE_CACHEDNODE_BIT
};

}

#endif
