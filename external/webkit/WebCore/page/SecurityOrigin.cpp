/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
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
#include "SecurityOrigin.h"

#include "CString.h"
#include "Document.h"
#include "KURL.h"
#include "OriginAccessEntry.h"
#include <wtf/StdLibExtras.h>

namespace WebCore {

static SecurityOrigin::LocalLoadPolicy localLoadPolicy = SecurityOrigin::AllowLocalLoadsForLocalOnly;

typedef Vector<OriginAccessEntry> OriginAccessWhiteList;
typedef HashMap<String, OriginAccessWhiteList*> OriginAccessMap;

static OriginAccessMap& originAccessMap()
{
    DEFINE_STATIC_LOCAL(OriginAccessMap, originAccessMap, ());
    return originAccessMap;
}

static URLSchemesMap& localSchemes()
{
    DEFINE_STATIC_LOCAL(URLSchemesMap, localSchemes, ());

    if (localSchemes.isEmpty()) {
        localSchemes.add("file");
#if PLATFORM(MAC)
        localSchemes.add("applewebdata");
#endif
#if PLATFORM(QT)
        localSchemes.add("qrc");
#endif
    }

    return localSchemes;
}

static URLSchemesMap& schemesWithUniqueOrigins()
{
    DEFINE_STATIC_LOCAL(URLSchemesMap, schemesWithUniqueOrigins, ());

    // This is a willful violation of HTML5.
    // See https://bugs.webkit.org/show_bug.cgi?id=11885
    if (schemesWithUniqueOrigins.isEmpty())
        schemesWithUniqueOrigins.add("data");

    return schemesWithUniqueOrigins;
}

SecurityOrigin::SecurityOrigin(const KURL& url, SandboxFlags sandboxFlags)
    : m_sandboxFlags(sandboxFlags)
    , m_protocol(url.protocol().isNull() ? "" : url.protocol().lower())
    , m_host(url.host().isNull() ? "" : url.host().lower())
    , m_port(url.port())
    , m_isUnique(isSandboxed(SandboxOrigin) || shouldTreatURLSchemeAsNoAccess(m_protocol))
    , m_universalAccess(false)
    , m_domainWasSetInDOM(false)
{
    // These protocols do not create security origins; the owner frame provides the origin
    if (m_protocol == "about" || m_protocol == "javascript")
        m_protocol = "";

    // document.domain starts as m_host, but can be set by the DOM.
    m_domain = m_host;

    // By default, only local SecurityOrigins can load local resources.
    m_canLoadLocalResources = isLocal();
    if (m_canLoadLocalResources) {
        // Directories should never be readable.
        if (!url.hasPath() || url.path().endsWith("/"))
            m_isUnique = true;
    }

    if (isDefaultPortForProtocol(m_port, m_protocol))
        m_port = 0;
}

SecurityOrigin::SecurityOrigin(const SecurityOrigin* other)
    : m_sandboxFlags(other->m_sandboxFlags)
    , m_protocol(other->m_protocol.threadsafeCopy())
    , m_host(other->m_host.threadsafeCopy())
    , m_domain(other->m_domain.threadsafeCopy())
    , m_port(other->m_port)
    , m_isUnique(other->m_isUnique)
    , m_universalAccess(other->m_universalAccess)
    , m_domainWasSetInDOM(other->m_domainWasSetInDOM)
    , m_canLoadLocalResources(other->m_canLoadLocalResources)
{
}

bool SecurityOrigin::isEmpty() const
{
    return m_protocol.isEmpty();
}

PassRefPtr<SecurityOrigin> SecurityOrigin::create(const KURL& url, SandboxFlags sandboxFlags)
{
    if (!url.isValid())
        return adoptRef(new SecurityOrigin(KURL(), sandboxFlags));
    return adoptRef(new SecurityOrigin(url, sandboxFlags));
}

PassRefPtr<SecurityOrigin> SecurityOrigin::createEmpty()
{
    return create(KURL());
}

PassRefPtr<SecurityOrigin> SecurityOrigin::threadsafeCopy()
{
    return adoptRef(new SecurityOrigin(this));
}

void SecurityOrigin::setDomainFromDOM(const String& newDomain)
{
    m_domainWasSetInDOM = true;
    m_domain = newDomain.lower();
}

static HashSet<String>& schemesForbiddenFromDomainRelaxation()
{
    DEFINE_STATIC_LOCAL(HashSet<String>, schemes, ());
    return schemes;
}

void SecurityOrigin::setDomainRelaxationForbiddenForURLScheme(bool forbidden, const String& scheme)
{
    if (scheme.isEmpty())
        return;

    if (forbidden)
        schemesForbiddenFromDomainRelaxation().add(scheme);
    else
        schemesForbiddenFromDomainRelaxation().remove(scheme);
}

bool SecurityOrigin::isDomainRelaxationForbiddenForURLScheme(const String& scheme)
{
    if (scheme.isEmpty())
        return false;

    return schemesForbiddenFromDomainRelaxation().contains(scheme);
}

bool SecurityOrigin::canAccess(const SecurityOrigin* other) const
{  
    if (m_universalAccess)
        return true;

    if (isUnique() || other->isUnique())
        return false;

    // Here are two cases where we should permit access:
    //
    // 1) Neither document has set document.domain. In this case, we insist
    //    that the scheme, host, and port of the URLs match.
    //
    // 2) Both documents have set document.domain. In this case, we insist
    //    that the documents have set document.domain to the same value and
    //    that the scheme of the URLs match.
    //
    // This matches the behavior of Firefox 2 and Internet Explorer 6.
    //
    // Internet Explorer 7 and Opera 9 are more strict in that they require
    // the port numbers to match when both pages have document.domain set.
    //
    // FIXME: Evaluate whether we can tighten this policy to require matched
    //        port numbers.
    //
    // Opera 9 allows access when only one page has set document.domain, but
    // this is a security vulnerability.

    if (m_protocol == other->m_protocol) {
        if (!m_domainWasSetInDOM && !other->m_domainWasSetInDOM) {
            if (m_host == other->m_host && m_port == other->m_port)
                return true;
        } else if (m_domainWasSetInDOM && other->m_domainWasSetInDOM) {
            if (m_domain == other->m_domain)
                return true;
        }
    }
    
    return false;
}

bool SecurityOrigin::canRequest(const KURL& url) const
{
    if (m_universalAccess)
        return true;

    if (isUnique())
        return false;

    RefPtr<SecurityOrigin> targetOrigin = SecurityOrigin::create(url);
    if (targetOrigin->isUnique())
        return false;

    // We call isSameSchemeHostPort here instead of canAccess because we want
    // to ignore document.domain effects.
    if (isSameSchemeHostPort(targetOrigin.get()))
        return true;

    if (OriginAccessWhiteList* list = originAccessMap().get(toString())) {
        for (size_t i = 0; i < list->size(); ++i) {
            if (list->at(i).matchesOrigin(*targetOrigin))
                return true;
        }
    }

    return false;
}

bool SecurityOrigin::taintsCanvas(const KURL& url) const
{
    if (canRequest(url))
        return false;

    // This function exists because we treat data URLs as having a unique origin,
    // contrary to the current (9/19/2009) draft of the HTML5 specification.
    // We still want to let folks paint data URLs onto untainted canvases, so
    // we special case data URLs below. If we change to match HTML5 w.r.t.
    // data URL security, then we can remove this function in favor of
    // !canRequest.
    if (url.protocolIs("data"))
        return false;

    return true;
}

bool SecurityOrigin::canLoad(const KURL& url, const String& referrer, Document* document)
{
    if (!shouldTreatURLAsLocal(url.string()))
        return true;

    // If we were provided a document, we let its local file policy dictate the result,
    // otherwise we allow local loads only if the supplied referrer is also local.
    if (document)
        return document->securityOrigin()->canLoadLocalResources();
    if (!referrer.isEmpty())
        return shouldTreatURLAsLocal(referrer);
    return false;
}

void SecurityOrigin::grantLoadLocalResources()
{
    // This function exists only to support backwards compatibility with older
    // versions of WebKit. Granting privileges to some, but not all, documents
    // in a SecurityOrigin is a security hazard because the documents without
    // the privilege can obtain the privilege by injecting script into the
    // documents that have been granted the privilege.
    ASSERT(allowSubstituteDataAccessToLocal());
    m_canLoadLocalResources = true;
}

void SecurityOrigin::grantUniversalAccess()
{
    m_universalAccess = true;
}

bool SecurityOrigin::isLocal() const
{
    return shouldTreatURLSchemeAsLocal(m_protocol);
}

bool SecurityOrigin::isSecureTransitionTo(const KURL& url) const
{ 
    // New window created by the application
    if (isEmpty())
        return true;

    RefPtr<SecurityOrigin> other = SecurityOrigin::create(url);
    return canAccess(other.get());
}

String SecurityOrigin::toString() const
{
    if (isEmpty())
        return "null";

    if (isUnique())
        return "null";

    if (m_protocol == "file")
        return String("file://");

    Vector<UChar> result;
    result.reserveInitialCapacity(m_protocol.length() + m_host.length() + 10);
    append(result, m_protocol);
    append(result, "://");
    append(result, m_host);

    if (m_port) {
        append(result, ":");
        append(result, String::number(m_port));
    }

    return String::adopt(result);
}

PassRefPtr<SecurityOrigin> SecurityOrigin::createFromString(const String& originString)
{
    return SecurityOrigin::create(KURL(KURL(), originString));
}

static const char SeparatorCharacter = '_';

PassRefPtr<SecurityOrigin> SecurityOrigin::createFromDatabaseIdentifier(const String& databaseIdentifier)
{ 
    // Make sure there's a first separator
    int separator1 = databaseIdentifier.find(SeparatorCharacter);
    if (separator1 == -1)
        return create(KURL());
        
    // Make sure there's a second separator
    int separator2 = databaseIdentifier.reverseFind(SeparatorCharacter);
    if (separator2 == -1)
        return create(KURL());
        
    // Ensure there were at least 2 separator characters. Some hostnames on intranets have
    // underscores in them, so we'll assume that any additional underscores are part of the host.
    if (separator1 == separator2)
        return create(KURL());
        
    // Make sure the port section is a valid port number or doesn't exist
    bool portOkay;
    int port = databaseIdentifier.right(databaseIdentifier.length() - separator2 - 1).toInt(&portOkay);
    bool portAbsent = (separator2 == static_cast<int>(databaseIdentifier.length()) - 1);
    if (!(portOkay || portAbsent))
        return create(KURL());
    
    if (port < 0 || port > 65535)
        return create(KURL());
        
    // Split out the 3 sections of data
    String protocol = databaseIdentifier.substring(0, separator1);
    String host = databaseIdentifier.substring(separator1 + 1, separator2 - separator1 - 1);
    return create(KURL(KURL(), protocol + "://" + host + ":" + String::number(port)));
}

String SecurityOrigin::databaseIdentifier() const 
{
    DEFINE_STATIC_LOCAL(String, separatorString, (&SeparatorCharacter, 1));
    return m_protocol + separatorString + m_host + separatorString + String::number(m_port); 
}

bool SecurityOrigin::equal(const SecurityOrigin* other) const 
{
    if (other == this)
        return true;
    
    if (!isSameSchemeHostPort(other))
        return false;

    if (m_domainWasSetInDOM != other->m_domainWasSetInDOM)
        return false;

    if (m_domainWasSetInDOM && m_domain != other->m_domain)
        return false;

    return true;
}

bool SecurityOrigin::isSameSchemeHostPort(const SecurityOrigin* other) const 
{
    if (m_host != other->m_host)
        return false;

    if (m_protocol != other->m_protocol)
        return false;

    if (m_port != other->m_port)
        return false;

    return true;
}

void SecurityOrigin::registerURLSchemeAsLocal(const String& scheme)
{
    localSchemes().add(scheme);
}

void SecurityOrigin::removeURLSchemeRegisteredAsLocal(const String& scheme)
{
    if (scheme == "file")
        return;
#if PLATFORM(MAC)
    if (scheme == "applewebdata")
        return;
#endif
    localSchemes().remove(scheme);
}

const URLSchemesMap& SecurityOrigin::localURLSchemes()
{
    return localSchemes();
}

bool SecurityOrigin::shouldTreatURLAsLocal(const String& url)
{
    // This avoids an allocation of another String and the HashSet contains()
    // call for the file: and http: schemes.
    if (url.length() >= 5) {
        const UChar* s = url.characters();
        if (s[0] == 'h' && s[1] == 't' && s[2] == 't' && s[3] == 'p' && s[4] == ':')
            return false;
        if (s[0] == 'f' && s[1] == 'i' && s[2] == 'l' && s[3] == 'e' && s[4] == ':')
            return true;
    }

    int loc = url.find(':');
    if (loc == -1)
        return false;

    String scheme = url.left(loc);
    return localSchemes().contains(scheme);
}

bool SecurityOrigin::shouldTreatURLSchemeAsLocal(const String& scheme)
{
    // This avoids an allocation of another String and the HashSet contains()
    // call for the file: and http: schemes.
    if (scheme.length() == 4) {
        const UChar* s = scheme.characters();
        if (s[0] == 'h' && s[1] == 't' && s[2] == 't' && s[3] == 'p')
            return false;
        if (s[0] == 'f' && s[1] == 'i' && s[2] == 'l' && s[3] == 'e')
            return true;
    }

    if (scheme.isEmpty())
        return false;

    return localSchemes().contains(scheme);
}

void SecurityOrigin::registerURLSchemeAsNoAccess(const String& scheme)
{
    schemesWithUniqueOrigins().add(scheme);
}

bool SecurityOrigin::shouldTreatURLSchemeAsNoAccess(const String& scheme)
{
    return schemesWithUniqueOrigins().contains(scheme);
}

bool SecurityOrigin::shouldHideReferrer(const KURL& url, const String& referrer)
{
    bool referrerIsSecureURL = protocolIs(referrer, "https");
    bool referrerIsWebURL = referrerIsSecureURL || protocolIs(referrer, "http");

    if (!referrerIsWebURL)
        return true;

    if (!referrerIsSecureURL)
        return false;

    bool URLIsSecureURL = url.protocolIs("https");

    return !URLIsSecureURL;
}

void SecurityOrigin::setLocalLoadPolicy(LocalLoadPolicy policy)
{
    localLoadPolicy = policy;
}

bool SecurityOrigin::restrictAccessToLocal()
{
    return localLoadPolicy != SecurityOrigin::AllowLocalLoadsForAll;
}

bool SecurityOrigin::allowSubstituteDataAccessToLocal()
{
    return localLoadPolicy != SecurityOrigin::AllowLocalLoadsForLocalOnly;
}

void SecurityOrigin::whiteListAccessFromOrigin(const SecurityOrigin& sourceOrigin, const String& destinationProtocol, const String& destinationDomains, bool allowDestinationSubdomains)
{
    ASSERT(isMainThread());
    ASSERT(!sourceOrigin.isEmpty());
    if (sourceOrigin.isEmpty())
        return;

    String sourceString = sourceOrigin.toString();
    OriginAccessWhiteList* list = originAccessMap().get(sourceString);
    if (!list) {
        list = new OriginAccessWhiteList;
        originAccessMap().set(sourceString, list);
    }
    list->append(OriginAccessEntry(destinationProtocol, destinationDomains, allowDestinationSubdomains ? OriginAccessEntry::AllowSubdomains : OriginAccessEntry::DisallowSubdomains));
}

void SecurityOrigin::resetOriginAccessWhiteLists()
{
    ASSERT(isMainThread());
    OriginAccessMap& map = originAccessMap();
    deleteAllValues(map);
    map.clear();
}

} // namespace WebCore
