/*
 * Copyright (C) 2003, 2008, 2010 Apple Inc. All rights reserved.
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
#include "JavaInstanceJSC.h"

#if ENABLE(MAC_JAVA_BRIDGE)

#include "JNIBridgeJSC.h"
#include "JNIUtility.h"
#include "JNIUtilityPrivate.h"
#include "JavaClassJSC.h"
#include "runtime_object.h"
#include "runtime_root.h"
#include <runtime/ArgList.h>
#include <runtime/Error.h>
#include <runtime/JSLock.h>

#if PLATFORM(ANDROID)
#include <assert.h>
#endif

#ifdef NDEBUG
#define JS_LOG(formatAndArgs...) ((void)0)
#else
#define JS_LOG(formatAndArgs...) { \
    fprintf(stderr, "%s:%d -- %s:  ", __FILE__, __LINE__, __FUNCTION__); \
    fprintf(stderr, formatAndArgs); \
}
#endif

#if PLATFORM(ANDROID)
#include <cutils/log.h>
#define LOG_TAG JavaInstanceJSC.cpp
#endif

using namespace JSC::Bindings;
using namespace JSC;

JavaInstance::JavaInstance(jobject instance, PassRefPtr<RootObject> rootObject)
    : Instance(rootObject)
{
    m_instance = new JObjectWrapper(instance);
    m_class = 0;
}

JavaInstance::~JavaInstance()
{
    delete m_class;
}

#define NUM_LOCAL_REFS 64

void JavaInstance::virtualBegin()
{
    getJNIEnv()->PushLocalFrame(NUM_LOCAL_REFS);
}

void JavaInstance::virtualEnd()
{
    getJNIEnv()->PopLocalFrame(0);
}

Class* JavaInstance::getClass() const
{
    if (!m_class)
        m_class = new JavaClass (m_instance->m_instance);
    return m_class;
}

JSValue JavaInstance::stringValue(ExecState* exec) const
{
    JSLock lock(SilenceAssertionsOnly);

    jstring stringValue = static_cast<jstring>(callJNIMethod_jobject(m_instance->m_instance, "toString", "()Ljava/lang/String;"));

    // Should throw a JS exception, rather than returning ""? - but better than a null dereference.
    if (!stringValue)
        return jsString(exec, UString());

    JNIEnv* env = getJNIEnv();
    const jchar* c = getUCharactersFromJStringInEnv(env, stringValue);
    UString u((const UChar*)c, (int)env->GetStringLength(stringValue));
    releaseUCharactersForJStringInEnv(env, stringValue, c);
    return jsString(exec, u);
}

JSValue JavaInstance::numberValue(ExecState* exec) const
{
    jdouble doubleValue = callJNIMethod<jdouble>(m_instance->m_instance, "doubleValue", "()D");
    return jsNumber(exec, doubleValue);
}

JSValue JavaInstance::booleanValue() const
{
    jboolean booleanValue = callJNIMethod<jboolean>(m_instance->m_instance, "booleanValue", "()Z");
    return jsBoolean(booleanValue);
}

JSValue JavaInstance::invokeMethod(ExecState* exec, const MethodList& methodList, const ArgList &args)
{
    int i, count = args.size();
    jvalue* jArgs;
    JSValue resultValue;
    Method* method = 0;
    size_t numMethods = methodList.size();

    // Try to find a good match for the overloaded method.  The
    // fundamental problem is that JavaScript doesn't have the
    // notion of method overloading and Java does.  We could
    // get a bit more sophisticated and attempt to does some
    // type checking as we as checking the number of parameters.
    Method* aMethod;
    for (size_t methodIndex = 0; methodIndex < numMethods; methodIndex++) {
        aMethod = methodList[methodIndex];
        if (aMethod->numParameters() == count) {
            method = aMethod;
            break;
        }
    }
    if (!method) {
        JS_LOG("unable to find an appropiate method\n");
        return jsUndefined();
    }

    const JavaMethod* jMethod = static_cast<const JavaMethod*>(method);
    JS_LOG("call %s %s on %p\n", UString(jMethod->name()).UTF8String().c_str(), jMethod->signature(), m_instance->m_instance);

    if (count > 0)
        jArgs = (jvalue*)malloc(count * sizeof(jvalue));
    else
        jArgs = 0;

    for (i = 0; i < count; i++) {
        JavaParameter* aParameter = jMethod->parameterAt(i);
        jArgs[i] = convertValueToJValue(exec, args.at(i), aParameter->getJNIType(), aParameter->type());
        JS_LOG("arg[%d] = %s\n", i, args.at(i).toString(exec).ascii());
    }

    jvalue result;

    // Try to use the JNI abstraction first, otherwise fall back to
    // normal JNI.  The JNI dispatch abstraction allows the Java plugin
    // to dispatch the call on the appropriate internal VM thread.
    RootObject* rootObject = this->rootObject();
    if (!rootObject)
        return jsUndefined();

    bool handled = false;
    if (rootObject->nativeHandle()) {
        jobject obj = m_instance->m_instance;
        JSValue exceptionDescription;
        const char *callingURL = 0; // FIXME, need to propagate calling URL to Java
        handled = dispatchJNICall(exec, rootObject->nativeHandle(), obj, jMethod->isStatic(), jMethod->JNIReturnType(), jMethod->methodID(obj), jArgs, result, callingURL, exceptionDescription);
        if (exceptionDescription) {
            throwError(exec, GeneralError, exceptionDescription.toString(exec));
            free(jArgs);
            return jsUndefined();
        }
    }

    // The following code can be conditionally removed once we have a Tiger update that
    // contains the new Java plugin.  It is needed for builds prior to Tiger.
    if (!handled) {
        jobject obj = m_instance->m_instance;
        switch (jMethod->JNIReturnType()) {
        case void_type:
            callJNIMethodIDA<void>(obj, jMethod->methodID(obj), jArgs);
            break;
        case object_type:
            result.l = callJNIMethodIDA<jobject>(obj, jMethod->methodID(obj), jArgs);
            break;
        case boolean_type:
            result.z = callJNIMethodIDA<jboolean>(obj, jMethod->methodID(obj), jArgs);
            break;
        case byte_type:
            result.b = callJNIMethodIDA<jbyte>(obj, jMethod->methodID(obj), jArgs);
            break;
        case char_type:
            result.c = callJNIMethodIDA<jchar>(obj, jMethod->methodID(obj), jArgs);
            break;
        case short_type:
            result.s = callJNIMethodIDA<jshort>(obj, jMethod->methodID(obj), jArgs);
            break;
        case int_type:
            result.i = callJNIMethodIDA<jint>(obj, jMethod->methodID(obj), jArgs);
            break;

        case long_type:
            result.j = callJNIMethodIDA<jlong>(obj, jMethod->methodID(obj), jArgs);
            break;
        case float_type:
            result.f = callJNIMethodIDA<jfloat>(obj, jMethod->methodID(obj), jArgs);
            break;
        case double_type:
            result.d = callJNIMethodIDA<jdouble>(obj, jMethod->methodID(obj), jArgs);
            break;
        case invalid_type:
        default:
            break;
        }
    }

    switch (jMethod->JNIReturnType()) {
    case void_type:
        {
            resultValue = jsUndefined();
        }
        break;

    case object_type:
        {
            if (result.l) {
                const char* arrayType = jMethod->returnType();
                if (arrayType[0] == '[')
                    resultValue = JavaArray::convertJObjectToArray(exec, result.l, arrayType, rootObject);
                else
                    resultValue = JavaInstance::create(result.l, rootObject)->createRuntimeObject(exec);
            } else
                resultValue = jsUndefined();
        }
        break;

    case boolean_type:
        {
            resultValue = jsBoolean(result.z);
        }
        break;

    case byte_type:
        {
            resultValue = jsNumber(exec, result.b);
        }
        break;

    case char_type:
        {
            resultValue = jsNumber(exec, result.c);
        }
        break;

    case short_type:
        {
            resultValue = jsNumber(exec, result.s);
        }
        break;

    case int_type:
        {
            resultValue = jsNumber(exec, result.i);
        }
        break;

    case long_type:
        {
            resultValue = jsNumber(exec, result.j);
        }
        break;

    case float_type:
        {
            resultValue = jsNumber(exec, result.f);
        }
        break;

    case double_type:
        {
            resultValue = jsNumber(exec, result.d);
        }
        break;

    case invalid_type:
    default:
        {
            resultValue = jsUndefined();
        }
        break;
    }

    free(jArgs);

    return resultValue;
}

JSValue JavaInstance::defaultValue(ExecState* exec, PreferredPrimitiveType hint) const
{
    if (hint == PreferString)
        return stringValue(exec);
    if (hint == PreferNumber)
        return numberValue(exec);
    JavaClass* aClass = static_cast<JavaClass*>(getClass());
    if (aClass->isStringClass())
        return stringValue(exec);
    if (aClass->isNumberClass())
        return numberValue(exec);
    if (aClass->isBooleanClass())
        return booleanValue();
    return valueOf(exec);
}

JSValue JavaInstance::valueOf(ExecState* exec) const
{
    return stringValue(exec);
}

JObjectWrapper::JObjectWrapper(jobject instance)
    : m_refCount(0)
{
    assert(instance);
#if PLATFORM(ANDROID)
    if (!instance)
        LOGE("Attempted to create JObjectWrapper for null object");
#endif

    // Cache the JNIEnv used to get the global ref for this java instance.
    // It'll be used to delete the reference.
    m_env = getJNIEnv();

    m_instance = m_env->NewGlobalRef(instance);

    JS_LOG("new global ref %p for %p\n", m_instance, instance);

    if  (!m_instance)
#if PLATFORM(ANDROID)
        LOGE("%s:  could not get GlobalRef for %p\n", __PRETTY_FUNCTION__, instance);
#else
        fprintf(stderr, "%s:  could not get GlobalRef for %p\n", __PRETTY_FUNCTION__, instance);
#endif
}

JObjectWrapper::~JObjectWrapper()
{
    JS_LOG("deleting global ref %p\n", m_instance);
    m_env->DeleteGlobalRef(m_instance);
}

#endif // ENABLE(MAC_JAVA_BRIDGE)
