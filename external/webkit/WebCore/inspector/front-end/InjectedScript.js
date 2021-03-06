/*
 * Copyright (C) 2007 Apple Inc.  All rights reserved.
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

var injectedScriptConstructor = (function (InjectedScriptHost, inspectedWindow, injectedScriptId) {

var InjectedScript = {};

InjectedScript.lastBoundObjectId = 1;
InjectedScript.idToWrappedObject = {};
InjectedScript.objectGroups = {};
InjectedScript.wrapObject = function(object, objectGroupName)
{
    var objectId;
    if (typeof object === "object" || typeof object === "function" ||
        (typeof object === "undefined" && object instanceof inspectedWindow.HTMLAllCollection)) { // FIXME(33716)
        var id = InjectedScript.lastBoundObjectId++;
        objectId = "object#" + id;
        InjectedScript.idToWrappedObject[objectId] = object;

        var group = InjectedScript.objectGroups[objectGroupName];
        if (!group) {
            group = [];
            InjectedScript.objectGroups[objectGroupName] = group;
        }
        group.push(objectId);
    }
    return InjectedScript.createProxyObject(object, objectId);
};

InjectedScript.unwrapObject = function(objectId) {
    return InjectedScript.idToWrappedObject[objectId];
};

InjectedScript.releaseWrapperObjectGroup = function(objectGroupName) {
    var group = InjectedScript.objectGroups[objectGroupName];
    if (!group)
        return;
    for (var i = 0; i < group.length; i++)
        delete InjectedScript.idToWrappedObject[group[i]];
    delete InjectedScript.objectGroups[objectGroupName];
};

// Called from within InspectorController on the 'inspected page' side.
InjectedScript.reset = function()
{
    InjectedScript._styles = {};
    InjectedScript._styleRules = {};
    InjectedScript._lastStyleId = 0;
    InjectedScript._lastStyleRuleId = 0;
    InjectedScript._searchResults = [];
    InjectedScript._includedInSearchResultsPropertyName = "__includedInInspectorSearchResults";
}

InjectedScript.reset();

InjectedScript.dispatch = function(methodName, args, callId)
{
    var argsArray = eval("(" + args + ")");
    if (callId)
        argsArray.splice(0, 0, callId);  // Methods that run asynchronously have a call back id parameter.
    var result = InjectedScript[methodName].apply(InjectedScript, argsArray);
    if (typeof result === "undefined") {
        InjectedScript._window().console.error("Web Inspector error: InjectedScript.%s returns undefined", methodName);
        result = null;
    }
    return result;
}

InjectedScript.getStyles = function(nodeId, authorOnly)
{
    var node = InjectedScript._nodeForId(nodeId);
    if (!node)
        return false;
    var defaultView = node.ownerDocument.defaultView;
    var matchedRules = defaultView.getMatchedCSSRules(node, "", authorOnly);
    var matchedCSSRules = [];
    for (var i = 0; matchedRules && i < matchedRules.length; ++i)
        matchedCSSRules.push(InjectedScript._serializeRule(matchedRules[i]));

    var styleAttributes = {};
    var attributes = node.attributes;
    for (var i = 0; attributes && i < attributes.length; ++i) {
        if (attributes[i].style)
            styleAttributes[attributes[i].name] = InjectedScript._serializeStyle(attributes[i].style, true);
    }
    var result = {};
    result.inlineStyle = InjectedScript._serializeStyle(node.style, true);
    result.computedStyle = InjectedScript._serializeStyle(defaultView.getComputedStyle(node));
    result.matchedCSSRules = matchedCSSRules;
    result.styleAttributes = styleAttributes;
    return result;
}

InjectedScript.getComputedStyle = function(nodeId)
{
    var node = InjectedScript._nodeForId(nodeId);
    if (!node)
        return false;
    return InjectedScript._serializeStyle(node.ownerDocument.defaultView.getComputedStyle(node));
}

InjectedScript.getInlineStyle = function(nodeId)
{
    var node = InjectedScript._nodeForId(nodeId);
    if (!node)
        return false;
    return InjectedScript._serializeStyle(node.style, true);
}

InjectedScript.applyStyleText = function(styleId, styleText, propertyName)
{
    var style = InjectedScript._styles[styleId];
    if (!style)
        return false;

    var styleTextLength = styleText.length;

    // Create a new element to parse the user input CSS.
    var parseElement = document.createElement("span");
    parseElement.setAttribute("style", styleText);

    var tempStyle = parseElement.style;
    if (tempStyle.length || !styleTextLength) {
        // The input was parsable or the user deleted everything, so remove the
        // original property from the real style declaration. If this represents
        // a shorthand remove all the longhand properties.
        if (style.getPropertyShorthand(propertyName)) {
            var longhandProperties = InjectedScript._getLonghandProperties(style, propertyName);
            for (var i = 0; i < longhandProperties.length; ++i)
                style.removeProperty(longhandProperties[i]);
        } else
            style.removeProperty(propertyName);
    }

    // Notify caller that the property was successfully deleted.
    if (!styleTextLength)
        return [null, [propertyName]];

    if (!tempStyle.length)
        return false;

    // Iterate of the properties on the test element's style declaration and
    // add them to the real style declaration. We take care to move shorthands.
    var foundShorthands = {};
    var changedProperties = [];
    var uniqueProperties = InjectedScript._getUniqueStyleProperties(tempStyle);
    for (var i = 0; i < uniqueProperties.length; ++i) {
        var name = uniqueProperties[i];
        var shorthand = tempStyle.getPropertyShorthand(name);

        if (shorthand && shorthand in foundShorthands)
            continue;

        if (shorthand) {
            var value = InjectedScript._getShorthandValue(tempStyle, shorthand);
            var priority = InjectedScript._getShorthandPriority(tempStyle, shorthand);
            foundShorthands[shorthand] = true;
        } else {
            var value = tempStyle.getPropertyValue(name);
            var priority = tempStyle.getPropertyPriority(name);
        }

        // Set the property on the real style declaration.
        style.setProperty((shorthand || name), value, priority);
        changedProperties.push(shorthand || name);
    }
    return [InjectedScript._serializeStyle(style, true), changedProperties];
}

InjectedScript.setStyleText = function(style, cssText)
{
    style.cssText = cssText;
    return true;
}

InjectedScript.toggleStyleEnabled = function(styleId, propertyName, disabled)
{
    var style = InjectedScript._styles[styleId];
    if (!style)
        return false;

    if (disabled) {
        if (!style.__disabledPropertyValues || !style.__disabledPropertyPriorities) {
            style.__disabledProperties = {};
            style.__disabledPropertyValues = {};
            style.__disabledPropertyPriorities = {};
        }

        style.__disabledPropertyValues[propertyName] = style.getPropertyValue(propertyName);
        style.__disabledPropertyPriorities[propertyName] = style.getPropertyPriority(propertyName);

        if (style.getPropertyShorthand(propertyName)) {
            var longhandProperties = InjectedScript._getLonghandProperties(style, propertyName);
            for (var i = 0; i < longhandProperties.length; ++i) {
                style.__disabledProperties[longhandProperties[i]] = true;
                style.removeProperty(longhandProperties[i]);
            }
        } else {
            style.__disabledProperties[propertyName] = true;
            style.removeProperty(propertyName);
        }
    } else if (style.__disabledProperties && style.__disabledProperties[propertyName]) {
        var value = style.__disabledPropertyValues[propertyName];
        var priority = style.__disabledPropertyPriorities[propertyName];

        style.setProperty(propertyName, value, priority);
        delete style.__disabledProperties[propertyName];
        delete style.__disabledPropertyValues[propertyName];
        delete style.__disabledPropertyPriorities[propertyName];
    }
    return InjectedScript._serializeStyle(style, true);
}

InjectedScript.applyStyleRuleText = function(ruleId, newContent, selectedNodeId)
{
    var rule = InjectedScript._styleRules[ruleId];
    if (!rule)
        return false;

    var selectedNode = InjectedScript._nodeForId(selectedNodeId);

    try {
        var stylesheet = rule.parentStyleSheet;
        stylesheet.addRule(newContent);
        var newRule = stylesheet.cssRules[stylesheet.cssRules.length - 1];
        newRule.style.cssText = rule.style.cssText;

        var parentRules = stylesheet.cssRules;
        for (var i = 0; i < parentRules.length; ++i) {
            if (parentRules[i] === rule) {
                rule.parentStyleSheet.removeRule(i);
                break;
            }
        }

        return [InjectedScript._serializeRule(newRule), InjectedScript._doesSelectorAffectNode(newContent, selectedNode)];
    } catch(e) {
        // Report invalid syntax.
        return false;
    }
}

InjectedScript.addStyleSelector = function(newContent, selectedNodeId)
{
    var selectedNode = InjectedScript._nodeForId(selectedNodeId);
    if (!selectedNode)
        return false;
    var ownerDocument = selectedNode.ownerDocument;

    var stylesheet = ownerDocument.__stylesheet;
    if (!stylesheet) {
        var head = ownerDocument.head;
        var styleElement = ownerDocument.createElement("style");
        styleElement.type = "text/css";
        head.appendChild(styleElement);
        stylesheet = ownerDocument.styleSheets[ownerDocument.styleSheets.length - 1];
        ownerDocument.__stylesheet = stylesheet;
    }

    try {
        stylesheet.addRule(newContent);
    } catch (e) {
        // Invalid Syntax for a Selector
        return false;
    }

    var rule = stylesheet.cssRules[stylesheet.cssRules.length - 1];
    rule.__isViaInspector = true;

    return [ InjectedScript._serializeRule(rule), InjectedScript._doesSelectorAffectNode(newContent, selectedNode) ];
}

InjectedScript._doesSelectorAffectNode = function(selectorText, node)
{
    if (!node)
        return false;
    var nodes = node.ownerDocument.querySelectorAll(selectorText);
    for (var i = 0; i < nodes.length; ++i) {
        if (nodes[i] === node) {
            return true;
        }
    }
    return false;
}

InjectedScript.setStyleProperty = function(styleId, name, value)
{
    var style = InjectedScript._styles[styleId];
    if (!style)
        return false;

    style.setProperty(name, value, "");
    return true;
}

InjectedScript._serializeRule = function(rule)
{
    var parentStyleSheet = rule.parentStyleSheet;

    var ruleValue = {};
    ruleValue.selectorText = rule.selectorText;
    if (parentStyleSheet) {
        ruleValue.parentStyleSheet = {};
        ruleValue.parentStyleSheet.href = parentStyleSheet.href;
    }
    ruleValue.isUserAgent = parentStyleSheet && !parentStyleSheet.ownerNode && !parentStyleSheet.href;
    ruleValue.isUser = parentStyleSheet && parentStyleSheet.ownerNode && parentStyleSheet.ownerNode.nodeName == "#document";
    ruleValue.isViaInspector = !!rule.__isViaInspector;

    // Bind editable scripts only.
    var doBind = !ruleValue.isUserAgent && !ruleValue.isUser;
    ruleValue.style = InjectedScript._serializeStyle(rule.style, doBind);

    if (doBind) {
        if (!rule.id) {
            rule.id = InjectedScript._lastStyleRuleId++;
            InjectedScript._styleRules[rule.id] = rule;
        }
        ruleValue.id = rule.id;
        ruleValue.injectedScriptId = injectedScriptId;
    }
    return ruleValue;
}

InjectedScript._serializeStyle = function(style, doBind)
{
    var result = {};
    result.width = style.width;
    result.height = style.height;
    result.__disabledProperties = style.__disabledProperties;
    result.__disabledPropertyValues = style.__disabledPropertyValues;
    result.__disabledPropertyPriorities = style.__disabledPropertyPriorities;
    result.properties = [];
    result.shorthandValues = {};
    var foundShorthands = {};
    for (var i = 0; i < style.length; ++i) {
        var property = {};
        var name = style[i];
        property.name = name;
        property.priority = style.getPropertyPriority(name);
        property.implicit = style.isPropertyImplicit(name);
        var shorthand =  style.getPropertyShorthand(name);
        property.shorthand = shorthand;
        if (shorthand && !(shorthand in foundShorthands)) {
            foundShorthands[shorthand] = true;
            result.shorthandValues[shorthand] = InjectedScript._getShorthandValue(style, shorthand);
        }
        property.value = style.getPropertyValue(name);
        result.properties.push(property);
    }
    result.uniqueStyleProperties = InjectedScript._getUniqueStyleProperties(style);

    if (doBind) {
        if (!style.id) {
            style.id = InjectedScript._lastStyleId++;
            InjectedScript._styles[style.id] = style;
        }
        result.id = style.id;
        result.injectedScriptId = injectedScriptId;
    }
    return result;
}

InjectedScript._getUniqueStyleProperties = function(style)
{
    var properties = [];
    var foundProperties = {};

    for (var i = 0; i < style.length; ++i) {
        var property = style[i];
        if (property in foundProperties)
            continue;
        foundProperties[property] = true;
        properties.push(property);
    }

    return properties;
}


InjectedScript._getLonghandProperties = function(style, shorthandProperty)
{
    var properties = [];
    var foundProperties = {};

    for (var i = 0; i < style.length; ++i) {
        var individualProperty = style[i];
        if (individualProperty in foundProperties || style.getPropertyShorthand(individualProperty) !== shorthandProperty)
            continue;
        foundProperties[individualProperty] = true;
        properties.push(individualProperty);
    }

    return properties;
}

InjectedScript._getShorthandValue = function(style, shorthandProperty)
{
    var value = style.getPropertyValue(shorthandProperty);
    if (!value) {
        // Some shorthands (like border) return a null value, so compute a shorthand value.
        // FIXME: remove this when http://bugs.webkit.org/show_bug.cgi?id=15823 is fixed.

        var foundProperties = {};
        for (var i = 0; i < style.length; ++i) {
            var individualProperty = style[i];
            if (individualProperty in foundProperties || style.getPropertyShorthand(individualProperty) !== shorthandProperty)
                continue;

            var individualValue = style.getPropertyValue(individualProperty);
            if (style.isPropertyImplicit(individualProperty) || individualValue === "initial")
                continue;

            foundProperties[individualProperty] = true;

            if (!value)
                value = "";
            else if (value.length)
                value += " ";
            value += individualValue;
        }
    }
    return value;
}

InjectedScript._getShorthandPriority = function(style, shorthandProperty)
{
    var priority = style.getPropertyPriority(shorthandProperty);
    if (!priority) {
        for (var i = 0; i < style.length; ++i) {
            var individualProperty = style[i];
            if (style.getPropertyShorthand(individualProperty) !== shorthandProperty)
                continue;
            priority = style.getPropertyPriority(individualProperty);
            break;
        }
    }
    return priority;
}

InjectedScript.getPrototypes = function(nodeId)
{
    var node = InjectedScript._nodeForId(nodeId);
    if (!node)
        return false;

    var result = [];
    for (var prototype = node; prototype; prototype = prototype.__proto__) {
        var title = InjectedScript._describe(prototype, true);
        if (title.match(/Prototype$/)) {
            title = title.replace(/Prototype$/, "");
        }
        result.push(title);
    }
    return result;
}

InjectedScript.getProperties = function(objectProxy, ignoreHasOwnProperty, abbreviate)
{
    var object = InjectedScript._resolveObject(objectProxy);
    if (!InjectedScript._isDefined(object))
        return false;

    var properties = [];

    // Go over properties, prepare results.
    for (var propertyName in object) {
        if (!ignoreHasOwnProperty && "hasOwnProperty" in object && !object.hasOwnProperty(propertyName))
            continue;

        var property = {};
        property.name = propertyName;
        property.parentObjectProxy = objectProxy;
        var isGetter = object["__lookupGetter__"] && object.__lookupGetter__(propertyName);
        if (!property.isGetter) {
            var childObject = object[propertyName];
            var childObjectProxy = new InjectedScript.createProxyObject(childObject, objectProxy.objectId, abbreviate);
            childObjectProxy.path = objectProxy.path ? objectProxy.path.slice() : [];
            childObjectProxy.path.push(propertyName);
            childObjectProxy.protoDepth = objectProxy.protoDepth || 0;
            property.value = childObjectProxy;
        } else {
            // FIXME: this should show something like "getter" (bug 16734).
            property.value = { description: "\u2014" }; // em dash
            property.isGetter = true;
        }
        properties.push(property);
    }
    return properties;
}

InjectedScript.setPropertyValue = function(objectProxy, propertyName, expression)
{
    var object = InjectedScript._resolveObject(objectProxy);
    if (!InjectedScript._isDefined(object))
        return false;

    var expressionLength = expression.length;
    if (!expressionLength) {
        delete object[propertyName];
        return !(propertyName in object);
    }

    try {
        // Surround the expression in parenthesis so the result of the eval is the result
        // of the whole expression not the last potential sub-expression.

        // There is a regression introduced here: eval is now happening against global object,
        // not call frame while on a breakpoint.
        // TODO: bring evaluation against call frame back.
        var result = InjectedScript._window().eval("(" + expression + ")");
        // Store the result in the property.
        object[propertyName] = result;
        return true;
    } catch(e) {
        try {
            var result = InjectedScript._window().eval("\"" + InjectedScript._escapeCharacters(expression, "\"") + "\"");
            object[propertyName] = result;
            return true;
        } catch(e) {
            return false;
        }
    }
}

InjectedScript.getNodePropertyValue = function(nodeId, propertyName)
{
    var node = InjectedScript._nodeForId(nodeId);
    if (!node)
        return false;
    var result = node[propertyName];
    return result !== undefined ? result : false;
}

InjectedScript.setOuterHTML = function(nodeId, value, expanded)
{
    var node = InjectedScript._nodeForId(nodeId);
    if (!node)
        return false;

    var parent = node.parentNode;
    var prevSibling = node.previousSibling;
    node.outerHTML = value;
    var newNode = prevSibling ? prevSibling.nextSibling : parent.firstChild;

    return InjectedScriptHost.pushNodePathToFrontend(newNode, expanded, false);
}

InjectedScript._getPropertyNames = function(object, resultSet)
{
    if (Object.getOwnPropertyNames) {
        for (var o = object; o; o = o.__proto__) {
            try {
                var names = Object.getOwnPropertyNames(o);
                for (var i = 0; i < names.length; ++i)
                    resultSet[names[i]] = true;
            } catch (e) {
            }
        }
    } else {
        // Chromium doesn't support getOwnPropertyNames yet.
        for (var name in object)
            resultSet[name] = true;
    }
}

InjectedScript.getCompletions = function(expression, includeInspectorCommandLineAPI, callFrameId)
{
    var props = {};
    try {
        var expressionResult;
        // Evaluate on call frame if call frame id is available.
        if (typeof callFrameId === "number") {
            var callFrame = InjectedScript._callFrameForId(callFrameId);
            if (!callFrame)
                return props;
            if (expression)
                expressionResult = InjectedScript._evaluateOn(callFrame.evaluate, callFrame, expression);
            else {
                // Evaluate into properties in scope of the selected call frame.
                var scopeChain = callFrame.scopeChain;
                for (var i = 0; i < scopeChain.length; ++i)
                    InjectedScript._getPropertyNames(scopeChain[i], props);
            }
        } else {
            if (!expression)
                expression = "this";
            expressionResult = InjectedScript._evaluateOn(InjectedScript._window().eval, InjectedScript._window(), expression);
        }
        if (typeof expressionResult == "object")
            InjectedScript._getPropertyNames(expressionResult, props);
        if (includeInspectorCommandLineAPI)
            for (var prop in InjectedScript._window().console._inspectorCommandLineAPI)
                if (prop.charAt(0) !== '_')
                    props[prop] = true;
    } catch(e) {
    }
    return props;
}

InjectedScript.evaluate = function(expression, objectGroup)
{
    return InjectedScript._evaluateAndWrap(InjectedScript._window().eval, InjectedScript._window(), expression, objectGroup);
}

InjectedScript._evaluateAndWrap = function(evalFunction, object, expression, objectGroup)
{
    var result = {};
    try {
        result.value = InjectedScript.wrapObject(InjectedScript._evaluateOn(evalFunction, object, expression), objectGroup);

        // Handle error that might have happened while describing result.
        if (result.value.errorText) {
            result.value = result.value.errorText;
            result.isException = true;
        }
    } catch (e) {
        result.value = e.toString();
        result.isException = true;
    }
    return result;
}

InjectedScript._evaluateOn = function(evalFunction, object, expression)
{
    InjectedScript._ensureCommandLineAPIInstalled(evalFunction, object);
    // Surround the expression in with statements to inject our command line API so that
    // the window object properties still take more precedent than our API functions.
    expression = "with (window.console._inspectorCommandLineAPI) { with (window) {\n" + expression + "\n} }";
    var value = evalFunction.call(object, expression);

    // When evaluating on call frame error is not thrown, but returned as a value.
    if (InjectedScript._type(value) === "error")
        throw value.toString();

    return value;
}

InjectedScript.addInspectedNode = function(nodeId)
{
    var node = InjectedScript._nodeForId(nodeId);
    if (!node)
        return false;

    InjectedScript._ensureCommandLineAPIInstalled(InjectedScript._window().eval, InjectedScript._window());
    var inspectedNodes = InjectedScript._window().console._inspectorCommandLineAPI._inspectedNodes;
    inspectedNodes.unshift(node);
    if (inspectedNodes.length >= 5)
        inspectedNodes.pop();
    return true;
}

InjectedScript.performSearch = function(whitespaceTrimmedQuery)
{
    var tagNameQuery = whitespaceTrimmedQuery;
    var attributeNameQuery = whitespaceTrimmedQuery;
    var startTagFound = (tagNameQuery.indexOf("<") === 0);
    var endTagFound = (tagNameQuery.lastIndexOf(">") === (tagNameQuery.length - 1));

    if (startTagFound || endTagFound) {
        var tagNameQueryLength = tagNameQuery.length;
        tagNameQuery = tagNameQuery.substring((startTagFound ? 1 : 0), (endTagFound ? (tagNameQueryLength - 1) : tagNameQueryLength));
    }

    // Check the tagNameQuery is it is a possibly valid tag name.
    if (!/^[a-zA-Z0-9\-_:]+$/.test(tagNameQuery))
        tagNameQuery = null;

    // Check the attributeNameQuery is it is a possibly valid tag name.
    if (!/^[a-zA-Z0-9\-_:]+$/.test(attributeNameQuery))
        attributeNameQuery = null;

    const escapedQuery = InjectedScript._escapeCharacters(whitespaceTrimmedQuery, "'");
    const escapedTagNameQuery = (tagNameQuery ?  InjectedScript._escapeCharacters(tagNameQuery, "'") : null);
    const escapedWhitespaceTrimmedQuery = InjectedScript._escapeCharacters(whitespaceTrimmedQuery, "'");
    const searchResultsProperty = InjectedScript._includedInSearchResultsPropertyName;

    function addNodesToResults(nodes, length, getItem)
    {
        if (!length)
            return;

        var nodeIds = [];
        for (var i = 0; i < length; ++i) {
            var node = getItem.call(nodes, i);
            // Skip this node if it already has the property.
            if (searchResultsProperty in node)
                continue;

            if (!InjectedScript._searchResults.length) {
                InjectedScript._currentSearchResultIndex = 0;
            }

            node[searchResultsProperty] = true;
            InjectedScript._searchResults.push(node);
            var nodeId = InjectedScriptHost.pushNodePathToFrontend(node, false, false);
            nodeIds.push(nodeId);
        }
        InjectedScriptHost.addNodesToSearchResult(nodeIds.join(","));
    }

    function matchExactItems(doc)
    {
        matchExactId.call(this, doc);
        matchExactClassNames.call(this, doc);
        matchExactTagNames.call(this, doc);
        matchExactAttributeNames.call(this, doc);
    }

    function matchExactId(doc)
    {
        const result = doc.__proto__.getElementById.call(doc, whitespaceTrimmedQuery);
        addNodesToResults.call(this, result, (result ? 1 : 0), function() { return this });
    }

    function matchExactClassNames(doc)
    {
        const result = doc.__proto__.getElementsByClassName.call(doc, whitespaceTrimmedQuery);
        addNodesToResults.call(this, result, result.length, result.item);
    }

    function matchExactTagNames(doc)
    {
        if (!tagNameQuery)
            return;
        const result = doc.__proto__.getElementsByTagName.call(doc, tagNameQuery);
        addNodesToResults.call(this, result, result.length, result.item);
    }

    function matchExactAttributeNames(doc)
    {
        if (!attributeNameQuery)
            return;
        const result = doc.__proto__.querySelectorAll.call(doc, "[" + attributeNameQuery + "]");
        addNodesToResults.call(this, result, result.length, result.item);
    }

    function matchPartialTagNames(doc)
    {
        if (!tagNameQuery)
            return;
        const result = doc.__proto__.evaluate.call(doc, "//*[contains(name(), '" + escapedTagNameQuery + "')]", doc, null, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE);
        addNodesToResults.call(this, result, result.snapshotLength, result.snapshotItem);
    }

    function matchStartOfTagNames(doc)
    {
        if (!tagNameQuery)
            return;
        const result = doc.__proto__.evaluate.call(doc, "//*[starts-with(name(), '" + escapedTagNameQuery + "')]", doc, null, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE);
        addNodesToResults.call(this, result, result.snapshotLength, result.snapshotItem);
    }

    function matchPartialTagNamesAndAttributeValues(doc)
    {
        if (!tagNameQuery) {
            matchPartialAttributeValues.call(this, doc);
            return;
        }

        const result = doc.__proto__.evaluate.call(doc, "//*[contains(name(), '" + escapedTagNameQuery + "') or contains(@*, '" + escapedQuery + "')]", doc, null, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE);
        addNodesToResults.call(this, result, result.snapshotLength, result.snapshotItem);
    }

    function matchPartialAttributeValues(doc)
    {
        const result = doc.__proto__.evaluate.call(doc, "//*[contains(@*, '" + escapedQuery + "')]", doc, null, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE);
        addNodesToResults.call(this, result, result.snapshotLength, result.snapshotItem);
    }

    function matchStyleSelector(doc)
    {
        const result = doc.__proto__.querySelectorAll.call(doc, whitespaceTrimmedQuery);
        addNodesToResults.call(this, result, result.length, result.item);
    }

    function matchPlainText(doc)
    {
        const result = doc.__proto__.evaluate.call(doc, "//text()[contains(., '" + escapedQuery + "')] | //comment()[contains(., '" + escapedQuery + "')]", doc, null, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE);
        addNodesToResults.call(this, result, result.snapshotLength, result.snapshotItem);
    }

    function matchXPathQuery(doc)
    {
        const result = doc.__proto__.evaluate.call(doc, whitespaceTrimmedQuery, doc, null, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE);
        addNodesToResults.call(this, result, result.snapshotLength, result.snapshotItem);
    }

    function finishedSearching()
    {
        // Remove the searchResultsProperty now that the search is finished.
        for (var i = 0; i < InjectedScript._searchResults.length; ++i)
            delete InjectedScript._searchResults[i][searchResultsProperty];
    }

    const mainFrameDocument = InjectedScript._window().document;
    const searchDocuments = [mainFrameDocument];
    var searchFunctions;
    if (tagNameQuery && startTagFound && endTagFound)
        searchFunctions = [matchExactTagNames, matchPlainText];
    else if (tagNameQuery && startTagFound)
        searchFunctions = [matchStartOfTagNames, matchPlainText];
    else if (tagNameQuery && endTagFound) {
        // FIXME: we should have a matchEndOfTagNames search function if endTagFound is true but not startTagFound.
        // This requires ends-with() support in XPath, WebKit only supports starts-with() and contains().
        searchFunctions = [matchPartialTagNames, matchPlainText];
    } else if (whitespaceTrimmedQuery === "//*" || whitespaceTrimmedQuery === "*") {
        // These queries will match every node. Matching everything isn't useful and can be slow for large pages,
        // so limit the search functions list to plain text and attribute matching.
        searchFunctions = [matchPartialAttributeValues, matchPlainText];
    } else
        searchFunctions = [matchExactItems, matchStyleSelector, matchPartialTagNamesAndAttributeValues, matchPlainText, matchXPathQuery];

    // Find all frames, iframes and object elements to search their documents.
    const querySelectorAllFunction = InjectedScript._window().Document.prototype.querySelectorAll;
    const subdocumentResult = querySelectorAllFunction.call(mainFrameDocument, "iframe, frame, object");

    for (var i = 0; i < subdocumentResult.length; ++i) {
        var element = subdocumentResult.item(i);
        if (element.contentDocument)
            searchDocuments.push(element.contentDocument);
    }

    const panel = InjectedScript;
    var documentIndex = 0;
    var searchFunctionIndex = 0;
    var chunkIntervalIdentifier = null;

    // Split up the work into chunks so we don't block the UI thread while processing.

    function processChunk()
    {
        var searchDocument = searchDocuments[documentIndex];
        var searchFunction = searchFunctions[searchFunctionIndex];

        if (++searchFunctionIndex > searchFunctions.length) {
            searchFunction = searchFunctions[0];
            searchFunctionIndex = 0;

            if (++documentIndex > searchDocuments.length) {
                if (panel._currentSearchChunkIntervalIdentifier === chunkIntervalIdentifier)
                    delete panel._currentSearchChunkIntervalIdentifier;
                clearInterval(chunkIntervalIdentifier);
                finishedSearching.call(panel);
                return;
            }

            searchDocument = searchDocuments[documentIndex];
        }

        if (!searchDocument || !searchFunction)
            return;

        try {
            searchFunction.call(panel, searchDocument);
        } catch(err) {
            // ignore any exceptions. the query might be malformed, but we allow that.
        }
    }

    processChunk();

    chunkIntervalIdentifier = setInterval(processChunk, 25);
    InjectedScript._currentSearchChunkIntervalIdentifier = chunkIntervalIdentifier;
    return true;
}

InjectedScript.searchCanceled = function()
{
    if (InjectedScript._searchResults) {
        const searchResultsProperty = InjectedScript._includedInSearchResultsPropertyName;
        for (var i = 0; i < this._searchResults.length; ++i) {
            var node = this._searchResults[i];

            // Remove the searchResultsProperty since there might be an unfinished search.
            delete node[searchResultsProperty];
        }
    }

    if (InjectedScript._currentSearchChunkIntervalIdentifier) {
        clearInterval(InjectedScript._currentSearchChunkIntervalIdentifier);
        delete InjectedScript._currentSearchChunkIntervalIdentifier;
    }
    InjectedScript._searchResults = [];
    return true;
}

InjectedScript.openInInspectedWindow = function(url)
{
    // Don't call window.open on wrapper - popup blocker mutes it.
    // URIs should have no double quotes.
    InjectedScript._window().eval("window.open(\"" + url + "\")");
    return true;
}

InjectedScript.callFrames = function()
{
    var callFrame = InjectedScriptHost.currentCallFrame();
    if (!callFrame)
        return false;

    var result = [];
    var depth = 0;
    do {
        result.push(new InjectedScript.CallFrameProxy(depth++, callFrame));
        callFrame = callFrame.caller;
    } while (callFrame);
    return result;
}

InjectedScript.evaluateInCallFrame = function(callFrameId, code, objectGroup)
{
    var callFrame = InjectedScript._callFrameForId(callFrameId);
    if (!callFrame)
        return false;
    return InjectedScript._evaluateAndWrap(callFrame.evaluate, callFrame, code, objectGroup);
}

InjectedScript._callFrameForId = function(id)
{
    var callFrame = InjectedScriptHost.currentCallFrame();
    while (--id >= 0 && callFrame)
        callFrame = callFrame.caller;
    return callFrame;
}

InjectedScript.clearConsoleMessages = function()
{
    InjectedScriptHost.clearConsoleMessages();
    return true;
}

InjectedScript._inspectObject = function(o)
{
    if (arguments.length === 0)
        return;

    inspectedWindow.console.log(o);
    if (InjectedScript._type(o) === "node") {
        InjectedScriptHost.pushNodePathToFrontend(o, false, true);
    } else {
        switch (InjectedScript._describe(o)) {
            case "Database":
                InjectedScriptHost.selectDatabase(o);
                break;
            case "Storage":
                InjectedScriptHost.selectDOMStorage(o);
                break;
        }
    }
}

InjectedScript._copy = function(o)
{
    if (InjectedScript._type(o) === "node") {
        var nodeId = InjectedScriptHost.pushNodePathToFrontend(o, false, false);
        InjectedScriptHost.copyNode(nodeId);
    } else {
        InjectedScriptHost.copyText(o);
    }
}

InjectedScript._ensureCommandLineAPIInstalled = function(evalFunction, evalObject)
{
    if (evalFunction.call(evalObject, "window.console._inspectorCommandLineAPI"))
        return;
    var inspectorCommandLineAPI = evalFunction.call(evalObject, "window.console._inspectorCommandLineAPI = { \n\
        $: function() { return document.getElementById.apply(document, arguments) }, \n\
        $$: function() { return document.querySelectorAll.apply(document, arguments) }, \n\
        $x: function(xpath, context) \n\
        { \n\
            var nodes = []; \n\
            try { \n\
                var doc = context || document; \n\
                var results = doc.evaluate(xpath, doc, null, XPathResult.ANY_TYPE, null); \n\
                var node; \n\
                while (node = results.iterateNext()) nodes.push(node); \n\
            } catch (e) {} \n\
            return nodes; \n\
        }, \n\
        dir: function() { return console.dir.apply(console, arguments) }, \n\
        dirxml: function() { return console.dirxml.apply(console, arguments) }, \n\
        keys: function(o) { var a = []; for (var k in o) a.push(k); return a; }, \n\
        values: function(o) { var a = []; for (var k in o) a.push(o[k]); return a; }, \n\
        profile: function() { return console.profile.apply(console, arguments) }, \n\
        profileEnd: function() { return console.profileEnd.apply(console, arguments) }, \n\
        _logEvent: function _inspectorCommandLineAPI_logEvent(e) { console.log(e.type, e); }, \n\
        _allEventTypes: [\"mouse\", \"key\", \"load\", \"unload\", \"abort\", \"error\", \n\
            \"select\", \"change\", \"submit\", \"reset\", \"focus\", \"blur\", \n\
            \"resize\", \"scroll\"], \n\
        _normalizeEventTypes: function(t) \n\
        { \n\
            if (typeof t === \"undefined\") \n\
                t = console._inspectorCommandLineAPI._allEventTypes; \n\
            else if (typeof t === \"string\") \n\
                t = [t]; \n\
            var i, te = []; \n\
            for (i = 0; i < t.length; i++) { \n\
                if (t[i] === \"mouse\") \n\
                    te.splice(0, 0, \"mousedown\", \"mouseup\", \"click\", \"dblclick\", \n\
                        \"mousemove\", \"mouseover\", \"mouseout\"); \n\
                else if (t[i] === \"key\") \n\
                    te.splice(0, 0, \"keydown\", \"keyup\", \"keypress\"); \n\
                else \n\
                    te.push(t[i]); \n\
            } \n\
            return te; \n\
        }, \n\
        monitorEvents: function(o, t) \n\
        { \n\
            if (!o || !o.addEventListener || !o.removeEventListener) \n\
                return; \n\
            t = console._inspectorCommandLineAPI._normalizeEventTypes(t); \n\
            for (i = 0; i < t.length; i++) { \n\
                o.removeEventListener(t[i], console._inspectorCommandLineAPI._logEvent, false); \n\
                o.addEventListener(t[i], console._inspectorCommandLineAPI._logEvent, false); \n\
            } \n\
        }, \n\
        unmonitorEvents: function(o, t) \n\
        { \n\
            if (!o || !o.removeEventListener) \n\
                return; \n\
            t = console._inspectorCommandLineAPI._normalizeEventTypes(t); \n\
            for (i = 0; i < t.length; i++) { \n\
                o.removeEventListener(t[i], console._inspectorCommandLineAPI._logEvent, false); \n\
            } \n\
        }, \n\
        _inspectedNodes: [], \n\
        get $0() { return console._inspectorCommandLineAPI._inspectedNodes[0] }, \n\
        get $1() { return console._inspectorCommandLineAPI._inspectedNodes[1] }, \n\
        get $2() { return console._inspectorCommandLineAPI._inspectedNodes[2] }, \n\
        get $3() { return console._inspectorCommandLineAPI._inspectedNodes[3] }, \n\
        get $4() { return console._inspectorCommandLineAPI._inspectedNodes[4] }, \n\
    };");

    inspectorCommandLineAPI.clear = InjectedScript.clearConsoleMessages;
    inspectorCommandLineAPI.inspect = InjectedScript._inspectObject;
    inspectorCommandLineAPI.copy = InjectedScript._copy;
}

InjectedScript._resolveObject = function(objectProxy)
{
    var object = InjectedScript._objectForId(objectProxy.objectId);
    var path = objectProxy.path;
    var protoDepth = objectProxy.protoDepth;

    // Follow the property path.
    for (var i = 0; InjectedScript._isDefined(object) && path && i < path.length; ++i)
        object = object[path[i]];

    // Get to the necessary proto layer.
    for (var i = 0; InjectedScript._isDefined(object) && protoDepth && i < protoDepth; ++i)
        object = object.__proto__;

    return object;
}

InjectedScript._window = function()
{
    // TODO: replace with 'return window;' once this script is injected into
    // the page's context.
    return inspectedWindow;
}

InjectedScript._nodeForId = function(nodeId)
{
    if (!nodeId)
        return null;
    return InjectedScriptHost.nodeForId(nodeId);
}

InjectedScript._objectForId = function(objectId)
{
    // There are three types of object ids used:
    // - numbers point to DOM Node via the InspectorDOMAgent mapping
    // - strings point to console objects cached in InspectorController for lazy evaluation upon them
    // - objects contain complex ids and are currently used for scoped objects
    if (typeof objectId === "number") {
        return InjectedScript._nodeForId(objectId);
    } else if (typeof objectId === "string") {
        return InjectedScript.unwrapObject(objectId);
    } else if (typeof objectId === "object") {
        var callFrame = InjectedScript._callFrameForId(objectId.callFrame);
        if (objectId.thisObject)
            return callFrame.thisObject;
        else
            return callFrame.scopeChain[objectId.chainIndex];
    }
    return objectId;
}

InjectedScript.pushNodeToFrontend = function(objectProxy)
{
    var object = InjectedScript._resolveObject(objectProxy);
    if (!object || InjectedScript._type(object) !== "node")
        return false;
    return InjectedScriptHost.pushNodePathToFrontend(object, false, false);
}

InjectedScript.nodeByPath = function(path)
{
    // We make this call through the injected script only to get a nice
    // callback for it.
    return InjectedScriptHost.pushNodeByPathToFrontend(path.join(","));
}

// Called from within InspectorController on the 'inspected page' side.
InjectedScript.createProxyObject = function(object, objectId, abbreviate)
{
    var result = {};
    result.injectedScriptId = injectedScriptId;
    result.objectId = objectId;
    result.type = InjectedScript._type(object);

    var type = typeof object;
    if ((type === "object" && object !== null) || type === "function") {
        for (var subPropertyName in object) {
            result.hasChildren = true;
            break;
        }
    }
    try {
        result.description = InjectedScript._describe(object, abbreviate);
    } catch (e) {
        result.errorText = e.toString();
    }
    return result;
}

InjectedScript.evaluateOnSelf = function(funcBody)
{
    return window.eval("(" + funcBody + ")();");
}

InjectedScript.CallFrameProxy = function(id, callFrame)
{
    this.id = id;
    this.type = callFrame.type;
    this.functionName = (this.type === "function" ? callFrame.functionName : "");
    this.sourceID = callFrame.sourceID;
    this.line = callFrame.line;
    this.scopeChain = this._wrapScopeChain(callFrame);
}

InjectedScript.CallFrameProxy.prototype = {
    _wrapScopeChain: function(callFrame)
    {
        var foundLocalScope = false;
        var scopeChain = callFrame.scopeChain;
        var scopeChainProxy = [];
        for (var i = 0; i < scopeChain.length; ++i) {
            var scopeObject = scopeChain[i];
            var scopeObjectProxy = InjectedScript.createProxyObject(scopeObject, { callFrame: this.id, chainIndex: i }, true);

            if (InjectedScriptHost.isActivation(scopeObject)) {
                if (!foundLocalScope)
                    scopeObjectProxy.thisObject = InjectedScript.createProxyObject(callFrame.thisObject, { callFrame: this.id, thisObject: true }, true);
                else
                    scopeObjectProxy.isClosure = true;
                foundLocalScope = true;
                scopeObjectProxy.isLocal = true;
            } else if (foundLocalScope && scopeObject instanceof InjectedScript._window().Element)
                scopeObjectProxy.isElement = true;
            else if (foundLocalScope && scopeObject instanceof InjectedScript._window().Document)
                scopeObjectProxy.isDocument = true;
            else if (!foundLocalScope)
                scopeObjectProxy.isWithBlock = true;
            scopeChainProxy.push(scopeObjectProxy);
        }
        return scopeChainProxy;
    }
}

InjectedScript.executeSql = function(callId, databaseId, query)
{
    function successCallback(tx, result)
    {
        var rows = result.rows;
        var result = [];
        var length = rows.length;
        for (var i = 0; i < length; ++i) {
            var data = {};
            result.push(data);
            var row = rows.item(i);
            for (var columnIdentifier in row) {
                // FIXME: (Bug 19439) We should specially format SQL NULL here
                // (which is represented by JavaScript null here, and turned
                // into the string "null" by the String() function).
                var text = row[columnIdentifier];
                data[columnIdentifier] = String(text);
            }
        }
        InjectedScriptHost.reportDidDispatchOnInjectedScript(callId, result, false);
    }

    function errorCallback(tx, error)
    {
        InjectedScriptHost.reportDidDispatchOnInjectedScript(callId, error, false);
    }

    function queryTransaction(tx)
    {
        tx.executeSql(query, null, successCallback, errorCallback);
    }

    var database = InjectedScriptHost.databaseForId(databaseId);
    if (!database)
        errorCallback(null, { code : 2 });  // Return as unexpected version.
    database.transaction(queryTransaction, errorCallback);
    return true;
}

InjectedScript._isDefined = function(object)
{
    return object || object instanceof inspectedWindow.HTMLAllCollection;
}

InjectedScript._type = function(obj)
{
    if (obj === null)
        return "null";

    // FIXME(33716): typeof document.all is always 'undefined'.
    if (obj instanceof inspectedWindow.HTMLAllCollection)
        return "array";

    var type = typeof obj;
    if (type !== "object" && type !== "function")
        return type;

    var win = InjectedScript._window();

    if (obj instanceof win.Node)
        return (obj.nodeType === undefined ? type : "node");
    if (obj instanceof win.String)
        return "string";
    if (obj instanceof win.Array)
        return "array";
    if (obj instanceof win.Boolean)
        return "boolean";
    if (obj instanceof win.Number)
        return "number";
    if (obj instanceof win.Date)
        return "date";
    if (obj instanceof win.RegExp)
        return "regexp";
    if (obj instanceof win.NodeList)
        return "array";
    if (obj instanceof win.HTMLCollection || obj instanceof win.HTMLAllCollection)
        return "array";
    if (obj instanceof win.Error)
        return "error";
    return type;
}

InjectedScript._describe = function(obj, abbreviated)
{
    var type1 = InjectedScript._type(obj);
    var type2 = InjectedScript._className(obj);

    switch (type1) {
    case "object":
    case "node":
    case "array":
        return type2;
    case "string":
        if (!abbreviated)
            return obj;
        if (obj.length > 100)
            return "\"" + obj.substring(0, 100) + "\u2026\"";
        return "\"" + obj + "\"";
    case "function":
        var objectText = String(obj);
        if (!/^function /.test(objectText))
            objectText = (type2 == "object") ? type1 : type2;
        else if (abbreviated)
            objectText = /.*/.exec(obj)[0].replace(/ +$/g, "");
        return objectText;
    default:
        return String(obj);
    }
}

InjectedScript._className = function(obj)
{
    return Object.prototype.toString.call(obj).replace(/^\[object (.*)\]$/i, "$1")
}

InjectedScript._escapeCharacters = function(str, chars)
{
    var foundChar = false;
    for (var i = 0; i < chars.length; ++i) {
        if (str.indexOf(chars.charAt(i)) !== -1) {
            foundChar = true;
            break;
        }
    }

    if (!foundChar)
        return str;

    var result = "";
    for (var i = 0; i < str.length; ++i) {
        if (chars.indexOf(str.charAt(i)) !== -1)
            result += "\\";
        result += str.charAt(i);
    }

    return result;
}

return InjectedScript;
});
