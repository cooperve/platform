/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

WebInspector.ScriptsPanel = function()
{
    WebInspector.Panel.call(this);

    this.element.addStyleClass("scripts");

    this.topStatusBar = document.createElement("div");
    this.topStatusBar.className = "status-bar";
    this.topStatusBar.id = "scripts-status-bar";
    this.element.appendChild(this.topStatusBar);

    this.backButton = document.createElement("button");
    this.backButton.className = "status-bar-item";
    this.backButton.id = "scripts-back";
    this.backButton.title = WebInspector.UIString("Show the previous script resource.");
    this.backButton.disabled = true;
    this.backButton.appendChild(document.createElement("img"));
    this.backButton.addEventListener("click", this._goBack.bind(this), false);
    this.topStatusBar.appendChild(this.backButton);

    this.forwardButton = document.createElement("button");
    this.forwardButton.className = "status-bar-item";
    this.forwardButton.id = "scripts-forward";
    this.forwardButton.title = WebInspector.UIString("Show the next script resource.");
    this.forwardButton.disabled = true;
    this.forwardButton.appendChild(document.createElement("img"));
    this.forwardButton.addEventListener("click", this._goForward.bind(this), false);
    this.topStatusBar.appendChild(this.forwardButton);

    this.filesSelectElement = document.createElement("select");
    this.filesSelectElement.className = "status-bar-item";
    this.filesSelectElement.id = "scripts-files";
    this.filesSelectElement.addEventListener("change", this._changeVisibleFile.bind(this), false);
    this.topStatusBar.appendChild(this.filesSelectElement);

    this.functionsSelectElement = document.createElement("select");
    this.functionsSelectElement.className = "status-bar-item";
    this.functionsSelectElement.id = "scripts-functions";

    // FIXME: append the functions select element to the top status bar when it is implemented.
    // this.topStatusBar.appendChild(this.functionsSelectElement);

    this.sidebarButtonsElement = document.createElement("div");
    this.sidebarButtonsElement.id = "scripts-sidebar-buttons";
    this.topStatusBar.appendChild(this.sidebarButtonsElement);

    this.pauseButton = document.createElement("button");
    this.pauseButton.className = "status-bar-item";
    this.pauseButton.id = "scripts-pause";
    this.pauseButton.title = WebInspector.UIString("Pause script execution.");
    this.pauseButton.disabled = true;
    this.pauseButton.appendChild(document.createElement("img"));
    this.pauseButton.addEventListener("click", this._togglePause.bind(this), false);
    this.sidebarButtonsElement.appendChild(this.pauseButton);

    this.stepOverButton = document.createElement("button");
    this.stepOverButton.className = "status-bar-item";
    this.stepOverButton.id = "scripts-step-over";
    this.stepOverButton.title = WebInspector.UIString("Step over next function call.");
    this.stepOverButton.disabled = true;
    this.stepOverButton.addEventListener("click", this._stepOverClicked.bind(this), false);
    this.stepOverButton.appendChild(document.createElement("img"));
    this.sidebarButtonsElement.appendChild(this.stepOverButton);

    this.stepIntoButton = document.createElement("button");
    this.stepIntoButton.className = "status-bar-item";
    this.stepIntoButton.id = "scripts-step-into";
    this.stepIntoButton.title = WebInspector.UIString("Step into next function call.");
    this.stepIntoButton.disabled = true;
    this.stepIntoButton.addEventListener("click", this._stepIntoClicked.bind(this), false);
    this.stepIntoButton.appendChild(document.createElement("img"));
    this.sidebarButtonsElement.appendChild(this.stepIntoButton);

    this.stepOutButton = document.createElement("button");
    this.stepOutButton.className = "status-bar-item";
    this.stepOutButton.id = "scripts-step-out";
    this.stepOutButton.title = WebInspector.UIString("Step out of current function.");
    this.stepOutButton.disabled = true;
    this.stepOutButton.addEventListener("click", this._stepOutClicked.bind(this), false);
    this.stepOutButton.appendChild(document.createElement("img"));
    this.sidebarButtonsElement.appendChild(this.stepOutButton);

    this.debuggerStatusElement = document.createElement("div");
    this.debuggerStatusElement.id = "scripts-debugger-status";
    this.sidebarButtonsElement.appendChild(this.debuggerStatusElement);

    this.viewsContainerElement = document.createElement("div");
    this.viewsContainerElement.id = "script-resource-views";

    this.sidebarElement = document.createElement("div");
    this.sidebarElement.id = "scripts-sidebar";

    this.sidebarResizeElement = document.createElement("div");
    this.sidebarResizeElement.className = "sidebar-resizer-vertical";
    this.sidebarResizeElement.addEventListener("mousedown", this._startSidebarResizeDrag.bind(this), false);

    this.sidebarResizeWidgetElement = document.createElement("div");
    this.sidebarResizeWidgetElement.id = "scripts-sidebar-resizer-widget";
    this.sidebarResizeWidgetElement.addEventListener("mousedown", this._startSidebarResizeDrag.bind(this), false);
    this.topStatusBar.appendChild(this.sidebarResizeWidgetElement);

    this.sidebarPanes = {};
    this.sidebarPanes.watchExpressions = new WebInspector.WatchExpressionsSidebarPane();
    this.sidebarPanes.callstack = new WebInspector.CallStackSidebarPane();
    this.sidebarPanes.scopechain = new WebInspector.ScopeChainSidebarPane();
    this.sidebarPanes.breakpoints = new WebInspector.BreakpointsSidebarPane();

    for (var pane in this.sidebarPanes)
        this.sidebarElement.appendChild(this.sidebarPanes[pane].element);

    this.sidebarPanes.callstack.expanded = true;
    this.sidebarPanes.callstack.addEventListener("call frame selected", this._callFrameSelected, this);

    this.sidebarPanes.scopechain.expanded = true;
    this.sidebarPanes.breakpoints.expanded = true;

    var panelEnablerHeading = WebInspector.UIString("You need to enable debugging before you can use the Scripts panel.");
    var panelEnablerDisclaimer = WebInspector.UIString("Enabling debugging will make scripts run slower.");
    var panelEnablerButton = WebInspector.UIString("Enable Debugging");

    this.panelEnablerView = new WebInspector.PanelEnablerView("scripts", panelEnablerHeading, panelEnablerDisclaimer, panelEnablerButton);
    this.panelEnablerView.addEventListener("enable clicked", this._enableDebugging, this);

    this.element.appendChild(this.panelEnablerView.element);
    this.element.appendChild(this.viewsContainerElement);
    this.element.appendChild(this.sidebarElement);
    this.element.appendChild(this.sidebarResizeElement);

    this.enableToggleButton = new WebInspector.StatusBarButton("", "enable-toggle-status-bar-item");
    this.enableToggleButton.addEventListener("click", this._toggleDebugging.bind(this), false);

    this.pauseOnExceptionButton = new WebInspector.StatusBarButton("", "scripts-pause-on-exceptions-status-bar-item", 3);
    this.pauseOnExceptionButton.addEventListener("click", this._togglePauseOnExceptions.bind(this), false);

    this._breakpointsURLMap = {};

    this._shortcuts = {};
    var handler, shortcut;
    var platformSpecificModifier = WebInspector.isMac() ? WebInspector.KeyboardShortcut.Modifiers.Meta : WebInspector.KeyboardShortcut.Modifiers.Ctrl;

    // Continue.
    handler = this.pauseButton.click.bind(this.pauseButton);
    shortcut = WebInspector.KeyboardShortcut.makeKey(WebInspector.KeyboardShortcut.KeyCodes.F8);
    this._shortcuts[shortcut] = handler;
    shortcut = WebInspector.KeyboardShortcut.makeKey(WebInspector.KeyboardShortcut.KeyCodes.Slash, platformSpecificModifier);
    this._shortcuts[shortcut] = handler;

    // Step over.
    handler = this.stepOverButton.click.bind(this.stepOverButton);
    shortcut = WebInspector.KeyboardShortcut.makeKey(WebInspector.KeyboardShortcut.KeyCodes.F10);
    this._shortcuts[shortcut] = handler;
    shortcut = WebInspector.KeyboardShortcut.makeKey(WebInspector.KeyboardShortcut.KeyCodes.SingleQuote, platformSpecificModifier);
    this._shortcuts[shortcut] = handler;

    // Step into.
    handler = this.stepIntoButton.click.bind(this.stepIntoButton);
    shortcut = WebInspector.KeyboardShortcut.makeKey(WebInspector.KeyboardShortcut.KeyCodes.F11);
    this._shortcuts[shortcut] = handler;
    shortcut = WebInspector.KeyboardShortcut.makeKey(WebInspector.KeyboardShortcut.KeyCodes.Semicolon, platformSpecificModifier);
    this._shortcuts[shortcut] = handler;

    // Step out.
    handler = this.stepOutButton.click.bind(this.stepOutButton);
    shortcut = WebInspector.KeyboardShortcut.makeKey(WebInspector.KeyboardShortcut.KeyCodes.F11, WebInspector.KeyboardShortcut.Modifiers.Shift);
    this._shortcuts[shortcut] = handler;
    shortcut = WebInspector.KeyboardShortcut.makeKey(WebInspector.KeyboardShortcut.KeyCodes.Semicolon, WebInspector.KeyboardShortcut.Modifiers.Shift, platformSpecificModifier);
    this._shortcuts[shortcut] = handler;

    this.reset();
}

// Keep these in sync with WebCore::JavaScriptDebugServer
WebInspector.ScriptsPanel.PauseOnExceptionsState = {
    DontPauseOnExceptions : 0,
    PauseOnAllExceptions : 1,
    PauseOnUncaughtExceptions: 2
};

WebInspector.ScriptsPanel.prototype = {
    toolbarItemClass: "scripts",

    get toolbarItemLabel()
    {
        return WebInspector.UIString("Scripts");
    },

    get statusBarItems()
    {
        return [this.enableToggleButton.element, this.pauseOnExceptionButton.element];
    },

    get defaultFocusedElement()
    {
        return this.filesSelectElement;
    },

    get paused()
    {
        return this._paused;
    },

    show: function()
    {
        WebInspector.Panel.prototype.show.call(this);
        this.sidebarResizeElement.style.right = (this.sidebarElement.offsetWidth - 3) + "px";

        if (this.visibleView) {
            if (this.visibleView instanceof WebInspector.ResourceView)
                this.visibleView.headersVisible = false;
            this.visibleView.show(this.viewsContainerElement);
        }
        // Hide any views that are visible that are not this panel's current visible view.
        // This can happen when a ResourceView is visible in the Resources panel then switched
        // to the this panel.
        for (var sourceID in this._sourceIDMap) {
            var scriptOrResource = this._sourceIDMap[sourceID];
            var view = this._sourceViewForScriptOrResource(scriptOrResource);
            if (!view || view === this.visibleView)
                continue;
            view.visible = false;
        }
        if (this._attachDebuggerWhenShown) {
            InspectorBackend.enableDebugger(false);
            delete this._attachDebuggerWhenShown;
        }
    },

    get searchableViews()
    {
        var views = [];

        const visibleView = this.visibleView;
        if (visibleView && visibleView.performSearch) {
            visibleView.alreadySearching = true;
            views.push(visibleView);
        }

        for (var sourceID in this._sourceIDMap) {
            var scriptOrResource = this._sourceIDMap[sourceID];
            var view = this._sourceViewForScriptOrResource(scriptOrResource);
            if (!view || !view.performSearch || view.alreadySearching)
                continue;

            view.alreadySearching = true;
            views.push(view);
        }

        for (var i = 0; i < views.length; ++i)
            delete views[i].alreadySearching;

        return views;
    },

    addScript: function(sourceID, sourceURL, source, startingLine, errorLine, errorMessage)
    {
        var script = new WebInspector.Script(sourceID, sourceURL, source, startingLine, errorLine, errorMessage);

        if (sourceURL in WebInspector.resourceURLMap) {
            var resource = WebInspector.resourceURLMap[sourceURL];
            resource.addScript(script);
        }

        sourceURL = script.sourceURL;

        if (sourceID)
            this._sourceIDMap[sourceID] = (resource || script);

        if (sourceURL in this._breakpointsURLMap && sourceID) {
            var breakpoints = this._breakpointsURLMap[sourceURL];
            var breakpointsLength = breakpoints.length;
            for (var i = 0; i < breakpointsLength; ++i) {
                var breakpoint = breakpoints[i];

                if (startingLine <= breakpoint.line) {
                    // remove and add the breakpoint, to clean up things like the sidebar
                    this.removeBreakpoint(breakpoint);
                    breakpoint.sourceID = sourceID;
                    this.addBreakpoint(breakpoint);
                    
                    if (breakpoint.enabled)
                        InspectorBackend.addBreakpoint(breakpoint.sourceID, breakpoint.line, breakpoint.condition);
                }
            }
        }

        this._addScriptToFilesMenu(script);
    },

    scriptOrResourceForID: function(id)
    {
        return this._sourceIDMap[id];
    },

    scriptForURL: function(url)
    {
        return this._scriptsForURLsInFilesSelect[url];
    },

    addBreakpoint: function(breakpoint)
    {
        this.sidebarPanes.breakpoints.addBreakpoint(breakpoint);

        var sourceFrame;
        if (breakpoint.url) {
            if (!(breakpoint.url in this._breakpointsURLMap))
                this._breakpointsURLMap[breakpoint.url] = [];
            this._breakpointsURLMap[breakpoint.url].unshift(breakpoint);

            if (breakpoint.url in WebInspector.resourceURLMap) {
                var resource = WebInspector.resourceURLMap[breakpoint.url];
                sourceFrame = this._sourceFrameForScriptOrResource(resource);
            }
        }

        if (breakpoint.sourceID && !sourceFrame) {
            var object = this._sourceIDMap[breakpoint.sourceID]
            sourceFrame = this._sourceFrameForScriptOrResource(object);
        }

        if (sourceFrame)
            sourceFrame.addBreakpoint(breakpoint);
    },

    removeBreakpoint: function(breakpoint)
    {
        this.sidebarPanes.breakpoints.removeBreakpoint(breakpoint);

        var sourceFrame;
        if (breakpoint.url && breakpoint.url in this._breakpointsURLMap) {
            var breakpoints = this._breakpointsURLMap[breakpoint.url];
            breakpoints.remove(breakpoint);
            if (!breakpoints.length)
                delete this._breakpointsURLMap[breakpoint.url];

            if (breakpoint.url in WebInspector.resourceURLMap) {
                var resource = WebInspector.resourceURLMap[breakpoint.url];
                sourceFrame = this._sourceFrameForScriptOrResource(resource);
            }
        }

        if (breakpoint.sourceID && !sourceFrame) {
            var object = this._sourceIDMap[breakpoint.sourceID]
            sourceFrame = this._sourceFrameForScriptOrResource(object);
        }

        if (sourceFrame)
            sourceFrame.removeBreakpoint(breakpoint);
    },

    selectedCallFrameId: function()
    {
        var selectedCallFrame = this.sidebarPanes.callstack.selectedCallFrame;
        if (!selectedCallFrame)
            return null;
        return selectedCallFrame.id;
    },

    evaluateInSelectedCallFrame: function(code, updateInterface, objectGroup, callback)
    {
        var selectedCallFrame = this.sidebarPanes.callstack.selectedCallFrame;
        if (!this._paused || !selectedCallFrame)
            return;

        if (typeof updateInterface === "undefined")
            updateInterface = true;

        var self = this;
        function updatingCallbackWrapper(result, exception)
        {
            callback(result, exception);
            if (updateInterface)
                self.sidebarPanes.scopechain.update(selectedCallFrame);
        }
        this.doEvalInCallFrame(selectedCallFrame, code, objectGroup, updatingCallbackWrapper);
    },

    doEvalInCallFrame: function(callFrame, code, objectGroup, callback)
    {
        function evalCallback(result)
        {
            if (result)
                callback(result.value, result.isException);
        }
        InjectedScriptAccess.get(callFrame.injectedScriptId).evaluateInCallFrame(callFrame.id, code, objectGroup, evalCallback);
    },

    debuggerPaused: function(callFrames)
    {
        this._paused = true;
        this._waitingToPause = false;
        this._stepping = false;

        this._updateDebuggerButtons();

        this.sidebarPanes.callstack.update(callFrames, this._sourceIDMap);
        this.sidebarPanes.callstack.selectedCallFrame = callFrames[0];

        WebInspector.currentPanel = this;
        window.focus();
    },

    debuggerResumed: function()
    {
        this._paused = false;
        this._waitingToPause = false;
        this._stepping = false;

        this._clearInterface();
    },

    attachDebuggerWhenShown: function()
    {
        if (this.element.parentElement) {
            InspectorBackend.enableDebugger(false);
        } else {
            this._attachDebuggerWhenShown = true;
        }
    },

    debuggerWasEnabled: function()
    {
        this.reset();
    },

    debuggerWasDisabled: function()
    {
        this.reset();
    },

    reset: function()
    {
        this.visibleView = null;

        delete this.currentQuery;
        this.searchCanceled();

        if (!InspectorBackend.debuggerEnabled()) {
            this._paused = false;
            this._waitingToPause = false;
            this._stepping = false;
        }

        this._clearInterface();

        this._backForwardList = [];
        this._currentBackForwardIndex = -1;
        this._updateBackAndForwardButtons();

        this._scriptsForURLsInFilesSelect = {};
        this.filesSelectElement.removeChildren();
        this.functionsSelectElement.removeChildren();
        this.viewsContainerElement.removeChildren();

        if (this._sourceIDMap) {
            for (var sourceID in this._sourceIDMap) {
                var object = this._sourceIDMap[sourceID];
                if (object instanceof WebInspector.Resource)
                    object.removeAllScripts();
            }
        }

        this._sourceIDMap = {};
        
        this.sidebarPanes.watchExpressions.refreshExpressions();
    },

    get visibleView()
    {
        return this._visibleView;
    },

    set visibleView(x)
    {
        if (this._visibleView === x)
            return;

        if (this._visibleView)
            this._visibleView.hide();

        this._visibleView = x;

        if (x)
            x.show(this.viewsContainerElement);
    },

    canShowSourceLineForURL: function(url)
    {
        return InspectorBackend.debuggerEnabled() &&
            !!(WebInspector.resourceForURL(url) || this.scriptForURL(url));
    },

    showSourceLineForURL: function(url, line)
    {
        var resource = WebInspector.resourceForURL(url);
        if (resource)
            this.showResource(resource, line);
        else
            this.showScript(this.scriptForURL(url), line);
    },

    showScript: function(script, line)
    {
        this._showScriptOrResource(script, {line: line, shouldHighlightLine: true});
    },

    showResource: function(resource, line)
    {
        this._showScriptOrResource(resource, {line: line, shouldHighlightLine: true});
    },

    showView: function(view)
    {
        if (!view)
            return;
        this._showScriptOrResource((view.resource || view.script));
    },

    handleShortcut: function(event)
    {
        var shortcut = WebInspector.KeyboardShortcut.makeKeyFromEvent(event);
        var handler = this._shortcuts[shortcut];
        if (handler) {
            handler(event);
            event.handled = true;
        } else
            this.sidebarPanes.callstack.handleShortcut(event);
    },

    scriptViewForScript: function(script)
    {
        if (!script)
            return null;
        if (!script._scriptView)
            script._scriptView = new WebInspector.ScriptView(script);
        return script._scriptView;
    },

    sourceFrameForScript: function(script)
    {
        var view = this.scriptViewForScript(script);
        if (!view)
            return null;

        // Setting up the source frame requires that we be attached.
        if (!this.element.parentNode)
            this.attach();

        view.setupSourceFrameIfNeeded();
        return view.sourceFrame;
    },

    _sourceViewForScriptOrResource: function(scriptOrResource)
    {
        if (scriptOrResource instanceof WebInspector.Resource) {
            if (!WebInspector.panels.resources)
                return null;
            return WebInspector.panels.resources.resourceViewForResource(scriptOrResource);
        }
        if (scriptOrResource instanceof WebInspector.Script)
            return this.scriptViewForScript(scriptOrResource);
    },

    _sourceFrameForScriptOrResource: function(scriptOrResource)
    {
        if (scriptOrResource instanceof WebInspector.Resource) {
            if (!WebInspector.panels.resources)
                return null;
            return WebInspector.panels.resources.sourceFrameForResource(scriptOrResource);
        }
        if (scriptOrResource instanceof WebInspector.Script)
            return this.sourceFrameForScript(scriptOrResource);
    },

    _showScriptOrResource: function(scriptOrResource, options)
    {
        // options = {line:, shouldHighlightLine:, fromBackForwardAction:, initialLoad:}
        if (!options) 
            options = {};

        if (!scriptOrResource)
            return;

        var view;
        if (scriptOrResource instanceof WebInspector.Resource) {
            if (!WebInspector.panels.resources)
                return null;
            view = WebInspector.panels.resources.resourceViewForResource(scriptOrResource);
            view.headersVisible = false; 

            if (scriptOrResource.url in this._breakpointsURLMap) {
                var sourceFrame = this._sourceFrameForScriptOrResource(scriptOrResource);
                if (sourceFrame && !sourceFrame.breakpoints.length) {
                    var breakpoints = this._breakpointsURLMap[scriptOrResource.url];
                    var breakpointsLength = breakpoints.length;
                    for (var i = 0; i < breakpointsLength; ++i)
                        sourceFrame.addBreakpoint(breakpoints[i]);
                }
            }
        } else if (scriptOrResource instanceof WebInspector.Script)
            view = this.scriptViewForScript(scriptOrResource);

        if (!view)
            return;

        var url = scriptOrResource.url || scriptOrResource.sourceURL;
        if (url && !options.initialLoad)
            WebInspector.settings.lastViewedScriptFile = url;

        if (!options.fromBackForwardAction) {
            var oldIndex = this._currentBackForwardIndex;
            if (oldIndex >= 0)
                this._backForwardList.splice(oldIndex + 1, this._backForwardList.length - oldIndex);

            // Check for a previous entry of the same object in _backForwardList.
            // If one is found, remove it and update _currentBackForwardIndex to match.
            var previousEntryIndex = this._backForwardList.indexOf(scriptOrResource);
            if (previousEntryIndex !== -1) {
                this._backForwardList.splice(previousEntryIndex, 1);
                --this._currentBackForwardIndex;
            }

            this._backForwardList.push(scriptOrResource);
            ++this._currentBackForwardIndex;

            this._updateBackAndForwardButtons();
        }

        this.visibleView = view;

        if (options.line) {
            if (view.revealLine)
                view.revealLine(options.line);
            if (view.highlightLine && options.shouldHighlightLine)
                view.highlightLine(options.line);
        }

        var option;
        if (scriptOrResource instanceof WebInspector.Script) {
            option = scriptOrResource.filesSelectOption;

            // hasn't been added yet - happens for stepping in evals,
            // so use the force option to force the script into the menu.
            if (!option) {
                this._addScriptToFilesMenu(scriptOrResource, {force: true});
                option = scriptOrResource.filesSelectOption;
            }

            console.assert(option);
        } else {
            var script = this.scriptForURL(url);
            if (script)
               option = script.filesSelectOption;
        }

        if (option)
            this.filesSelectElement.selectedIndex = option.index;
    },

    _addScriptToFilesMenu: function(script, options)
    {
        var force = options && options.force;

        if (!script.sourceURL && !force)
            return;

        if (script.resource && this._scriptsForURLsInFilesSelect[script.sourceURL])
            return;

        this._scriptsForURLsInFilesSelect[script.sourceURL] = script;

        var select = this.filesSelectElement;

        var option = document.createElement("option");
        option.representedObject = (script.resource || script);
        option.text = (script.sourceURL ? WebInspector.displayNameForURL(script.sourceURL) : WebInspector.UIString("(program)"));

        function optionCompare(a, b)
        {
            var aTitle = a.text.toLowerCase();
            var bTitle = b.text.toLowerCase();
            if (aTitle < bTitle)
                return -1;
            else if (aTitle > bTitle)
                return 1;

            var aSourceID = a.representedObject.sourceID;
            var bSourceID = b.representedObject.sourceID;
            if (aSourceID < bSourceID)
                return -1;
            else if (aSourceID > bSourceID)
                return 1;
            return 0;
        }

        var insertionIndex = insertionIndexForObjectInListSortedByFunction(option, select.childNodes, optionCompare);
        if (insertionIndex < 0)
            select.appendChild(option);
        else
            select.insertBefore(option, select.childNodes.item(insertionIndex));

        script.filesSelectOption = option;

        // Call _showScriptOrResource if the option we just appended ended up being selected.
        // This will happen for the first item added to the menu.
        if (select.options[select.selectedIndex] === option)
            this._showScriptOrResource(option.representedObject, {initialLoad: true});
        else {
            // if not first item, check to see if this was the last viewed
            var url = option.representedObject.url || option.representedObject.sourceURL;
            var lastURL = WebInspector.settings.lastViewedScriptFile;
            if (url && url === lastURL)
                this._showScriptOrResource(option.representedObject, {initialLoad: true});
        }
    },

    _clearCurrentExecutionLine: function()
    {
        if (this._executionSourceFrame)
            this._executionSourceFrame.executionLine = 0;
        delete this._executionSourceFrame;
    },

    _callFrameSelected: function()
    {
        this._clearCurrentExecutionLine();

        var callStackPane = this.sidebarPanes.callstack;
        var currentFrame = callStackPane.selectedCallFrame;
        if (!currentFrame)
            return;

        this.sidebarPanes.scopechain.update(currentFrame);
        this.sidebarPanes.watchExpressions.refreshExpressions();

        var scriptOrResource = this._sourceIDMap[currentFrame.sourceID];
        this._showScriptOrResource(scriptOrResource, {line: currentFrame.line});

        this._executionSourceFrame = this._sourceFrameForScriptOrResource(scriptOrResource);
        if (this._executionSourceFrame)
            this._executionSourceFrame.executionLine = currentFrame.line;
    },

    _changeVisibleFile: function(event)
    {
        var select = this.filesSelectElement;
        this._showScriptOrResource(select.options[select.selectedIndex].representedObject);
    },

    _startSidebarResizeDrag: function(event)
    {
        WebInspector.elementDragStart(this.sidebarElement, this._sidebarResizeDrag.bind(this), this._endSidebarResizeDrag.bind(this), event, "col-resize");

        if (event.target === this.sidebarResizeWidgetElement)
            this._dragOffset = (event.target.offsetWidth - (event.pageX - event.target.totalOffsetLeft));
        else
            this._dragOffset = 0;
    },

    _endSidebarResizeDrag: function(event)
    {
        WebInspector.elementDragEnd(event);

        delete this._dragOffset;
    },

    _sidebarResizeDrag: function(event)
    {
        var x = event.pageX + this._dragOffset;
        var newWidth = Number.constrain(window.innerWidth - x, Preferences.minScriptsSidebarWidth, window.innerWidth * 0.66);

        this.sidebarElement.style.width = newWidth + "px";
        this.sidebarButtonsElement.style.width = newWidth + "px";
        this.viewsContainerElement.style.right = newWidth + "px";
        this.sidebarResizeWidgetElement.style.right = newWidth + "px";
        this.sidebarResizeElement.style.right = (newWidth - 3) + "px";

        this.resize();
        event.preventDefault();
    },
    
    _updatePauseOnExceptionsButton: function()
    {
        if (InspectorBackend.pauseOnExceptionsState() == WebInspector.ScriptsPanel.PauseOnExceptionsState.DontPauseOnExceptions)
            this.pauseOnExceptionButton.title = WebInspector.UIString("Don't pause on exceptions.\nClick to Pause on all exceptions.");
        else if (InspectorBackend.pauseOnExceptionsState() == WebInspector.ScriptsPanel.PauseOnExceptionsState.PauseOnAllExceptions)
            this.pauseOnExceptionButton.title = WebInspector.UIString("Pause on all exceptions.\nClick to Pause on uncaught exceptions.");
        else if (InspectorBackend.pauseOnExceptionsState() == WebInspector.ScriptsPanel.PauseOnExceptionsState.PauseOnUncaughtExceptions)
            this.pauseOnExceptionButton.title = WebInspector.UIString("Pause on uncaught exceptions.\nClick to Not pause on exceptions.");
        
        this.pauseOnExceptionButton.state = InspectorBackend.pauseOnExceptionsState();
        
    },

    _updateDebuggerButtons: function()
    {
        if (InspectorBackend.debuggerEnabled()) {
            this.enableToggleButton.title = WebInspector.UIString("Debugging enabled. Click to disable.");
            this.enableToggleButton.toggled = true;
            this.pauseOnExceptionButton.visible = true;
            this.panelEnablerView.visible = false;
        } else {
            this.enableToggleButton.title = WebInspector.UIString("Debugging disabled. Click to enable.");
            this.enableToggleButton.toggled = false;
            this.pauseOnExceptionButton.visible = false;
            this.panelEnablerView.visible = true;
        }

        this._updatePauseOnExceptionsButton();

        if (this._paused) {
            this.pauseButton.addStyleClass("paused");

            this.pauseButton.disabled = false;
            this.stepOverButton.disabled = false;
            this.stepIntoButton.disabled = false;
            this.stepOutButton.disabled = false;

            this.debuggerStatusElement.textContent = WebInspector.UIString("Paused");
        } else {
            this.pauseButton.removeStyleClass("paused");

            this.pauseButton.disabled = this._waitingToPause;
            this.stepOverButton.disabled = true;
            this.stepIntoButton.disabled = true;
            this.stepOutButton.disabled = true;

            if (this._waitingToPause)
                this.debuggerStatusElement.textContent = WebInspector.UIString("Pausing");
            else if (this._stepping)
                this.debuggerStatusElement.textContent = WebInspector.UIString("Stepping");
            else
                this.debuggerStatusElement.textContent = "";
        }
    },

    _updateBackAndForwardButtons: function()
    {
        this.backButton.disabled = this._currentBackForwardIndex <= 0;
        this.forwardButton.disabled = this._currentBackForwardIndex >= (this._backForwardList.length - 1);
    },

    _clearInterface: function()
    {
        this.sidebarPanes.callstack.update(null);
        this.sidebarPanes.scopechain.update(null);

        this._clearCurrentExecutionLine();
        this._updateDebuggerButtons();
    },

    _goBack: function()
    {
        if (this._currentBackForwardIndex <= 0) {
            console.error("Can't go back from index " + this._currentBackForwardIndex);
            return;
        }

        this._showScriptOrResource(this._backForwardList[--this._currentBackForwardIndex], {fromBackForwardAction: true});
        this._updateBackAndForwardButtons();
    },

    _goForward: function()
    {
        if (this._currentBackForwardIndex >= this._backForwardList.length - 1) {
            console.error("Can't go forward from index " + this._currentBackForwardIndex);
            return;
        }

        this._showScriptOrResource(this._backForwardList[++this._currentBackForwardIndex], {fromBackForwardAction: true});
        this._updateBackAndForwardButtons();
    },

    _enableDebugging: function()
    {
        if (InspectorBackend.debuggerEnabled())
            return;
        this._toggleDebugging(this.panelEnablerView.alwaysEnabled);
    },

    _toggleDebugging: function(optionalAlways)
    {
        this._paused = false;
        this._waitingToPause = false;
        this._stepping = false;

        if (InspectorBackend.debuggerEnabled())
            InspectorBackend.disableDebugger(true);
        else
            InspectorBackend.enableDebugger(!!optionalAlways);
    },

    _togglePauseOnExceptions: function()
    {
        InspectorBackend.setPauseOnExceptionsState((InspectorBackend.pauseOnExceptionsState() + 1) % this.pauseOnExceptionButton.states);
        this._updatePauseOnExceptionsButton();
    },

    _togglePause: function()
    {
        if (this._paused) {
            this._paused = false;
            this._waitingToPause = false;
            InspectorBackend.resumeDebugger();
        } else {
            this._stepping = false;
            this._waitingToPause = true;
            InspectorBackend.pauseInDebugger();
        }

        this._clearInterface();
    },

    _stepOverClicked: function()
    {
        this._paused = false;
        this._stepping = true;

        this._clearInterface();

        InspectorBackend.stepOverStatementInDebugger();
    },

    _stepIntoClicked: function()
    {
        this._paused = false;
        this._stepping = true;

        this._clearInterface();

        InspectorBackend.stepIntoStatementInDebugger();
    },

    _stepOutClicked: function()
    {
        this._paused = false;
        this._stepping = true;

        this._clearInterface();

        InspectorBackend.stepOutOfFunctionInDebugger();
    }
}

WebInspector.ScriptsPanel.prototype.__proto__ = WebInspector.Panel.prototype;
