/*
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 *
 * All rights reserved.
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
#include "CookieJar.h"

#include "Cookie.h"
#include "Document.h"
#include "KURL.h"
#include "PlatformString.h"

#include "qwebpage.h"
#include "qwebframe.h"
#include "FrameLoaderClientQt.h"
#include <QNetworkAccessManager>
#include <QNetworkCookie>

namespace WebCore {

static QNetworkCookieJar *cookieJar(const Document *document)
{
    if (!document)
        return 0;
    Frame *frame = document->frame();
    if (!frame)
        return 0;
    FrameLoader *loader = frame->loader();
    if (!loader)
        return 0;
    QWebFrame* webFrame = static_cast<FrameLoaderClientQt*>(loader->client())->webFrame();
    QWebPage* page = webFrame->page();
    QNetworkAccessManager* manager = page->networkAccessManager();
    QNetworkCookieJar* jar = manager->cookieJar();
    return jar;
}

void setCookies(Document* document, const KURL& url, const String& value)
{
    QUrl u(url);
    QUrl p(document->firstPartyForCookies());
    QNetworkCookieJar* jar = cookieJar(document);
    if (!jar)
        return;

    QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(QString(value).toAscii());
#if QT_VERSION >= 0x040500
    QList<QNetworkCookie>::Iterator it = cookies.begin();
    while (it != cookies.end()) {
        if (it->isHttpOnly())
            it = cookies.erase(it);
        else
            ++it;
    }
#endif
    jar->setCookiesFromUrl(cookies, u);
}

String cookies(const Document* document, const KURL& url)
{
    QUrl u(url);
    QNetworkCookieJar* jar = cookieJar(document);
    if (!jar)
        return String();

    QList<QNetworkCookie> cookies = jar->cookiesForUrl(u);
    if (cookies.isEmpty())
        return String();

    QStringList resultCookies;
    foreach (QNetworkCookie networkCookie, cookies) {
#if QT_VERSION >= 0x040500
        if (networkCookie.isHttpOnly())
            continue;
#endif
        resultCookies.append(QString::fromAscii(
                             networkCookie.toRawForm(QNetworkCookie::NameAndValueOnly).constData()));
    }

    return resultCookies.join(QLatin1String("; "));
}

String cookieRequestHeaderFieldValue(const Document* document, const KURL &url)
{
    QUrl u(url);
    QNetworkCookieJar* jar = cookieJar(document);
    if (!jar)
        return String();

    QList<QNetworkCookie> cookies = jar->cookiesForUrl(u);
    if (cookies.isEmpty())
        return String();

    QStringList resultCookies;
    foreach (QNetworkCookie networkCookie, cookies) {
        resultCookies.append(QString::fromAscii(
                             networkCookie.toRawForm(QNetworkCookie::NameAndValueOnly).constData()));
    }

    return resultCookies.join(QLatin1String("; "));
}

bool cookiesEnabled(const Document* document)
{
    QNetworkCookieJar* jar = cookieJar(document);
    return (jar != 0);
}

bool getRawCookies(const Document*, const KURL&, Vector<Cookie>& rawCookies)
{
    // FIXME: Not yet implemented
    rawCookies.clear();
    return false; // return true when implemented
}

void deleteCookie(const Document*, const KURL&, const String&)
{
    // FIXME: Not yet implemented
}

}

// vim: ts=4 sw=4 et
