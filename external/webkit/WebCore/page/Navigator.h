/*
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef Navigator_h
#define Navigator_h

#include "NavigatorBase.h"
#include "PlatformString.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>

namespace WebCore {

    class Frame;
    class Geolocation;
    class MimeTypeArray;
    class PluginData;
    class PluginArray;
#if PLATFORM(ANDROID)
    class ApplicationInstalledCallback;
    class Connection;
#endif


    typedef int ExceptionCode;

    class Navigator : public NavigatorBase, public RefCounted<Navigator> {
    public:
        static PassRefPtr<Navigator> create(Frame* frame) { return adoptRef(new Navigator(frame)); }
        ~Navigator();

        void disconnectFrame();
        Frame* frame() const { return m_frame; }

        String appVersion() const;
        String language() const;
        PluginArray* plugins() const;
        MimeTypeArray* mimeTypes() const;
        bool cookieEnabled() const;
        bool javaEnabled() const;

        virtual String userAgent() const;

        Geolocation* geolocation() const;
        // This is used for GC marking.
        Geolocation* optionalGeolocation() const { return m_geolocation.get(); }

#if PLATFORM(ANDROID)
        Connection* connection() const;
#endif

#if PLATFORM(ANDROID) && ENABLE(APPLICATION_INSTALLED)
        bool isApplicationInstalled(const String& name, PassRefPtr<ApplicationInstalledCallback> callback);
        void onPackageResult();
#endif

#if ENABLE(DOM_STORAGE)
        // Relinquishes the storage lock, if one exists.
        void getStorageUpdates();
#endif

        void registerProtocolHandler(const String& scheme, const String& url, const String& title, ExceptionCode& ec);
        void registerContentHandler(const String& mimeType, const String& url, const String& title, ExceptionCode& ec);

    private:
        Navigator(Frame*);
        Frame* m_frame;
        mutable RefPtr<PluginArray> m_plugins;
        mutable RefPtr<MimeTypeArray> m_mimeTypes;
        mutable RefPtr<Geolocation> m_geolocation;
#if PLATFORM(ANDROID)
        mutable RefPtr<Connection> m_connection;
#endif

#if PLATFORM(ANDROID) && ENABLE(APPLICATION_INSTALLED)
        RefPtr<ApplicationInstalledCallback> m_applicationInstalledCallback;
        String m_applicationNameQuery;
#endif
    };

}

#endif
