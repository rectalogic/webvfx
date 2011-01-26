// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromix/chromix.h"
#include "chromix/chromix_extension.h"
#include "chromix/webkit_client_impl.h"

#include <base/message_loop.h>
#include <base/at_exit.h>
#include <base/command_line.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebKit.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebRuntimeFeatures.h>
#include <third_party/WebKit/Source/WebKit/chromium/public/WebScriptController.h>
#include <webkit/tools/test_shell/simple_resource_loader_bridge.h>
#if defined(OS_MACOSX)
#include <third_party/WebKit/Source/WebKit/mac/WebCoreSupport/WebSystemInterface.h>
#elif defined(OS_LINUX)
#include <app/app_switches.h>
#include <app/gfx/gl/gl_implementation.h>
#include <gfx/gtk_util.h>
#endif

namespace Chromix {

class MixKit {
public:
    // webKitClient depends on state initialized by messageLoop
    MixKit(int argc, const char* argv[]) :
        atExitManager(),
        messageLoop(),
        webKitClient()
    {
        CommandLine::Init(argc, argv);

        WebKit::initialize(&webKitClient);

#if defined(OS_MACOSX)
        InitWebCoreSystemInterface();
#elif defined(OS_LINUX)
        //XXX should we use defined(TOOLKIT_USES_GTK) ?
        // Check if --use-gl specified
        // If 'osmesa' specified, then explicitly initialize it and don't initialize
        // X11. If we don't explicitly initialize it then we will crash
        // because the desktop GL will be tried first and we won't have an X connection.
        // For anything other than 'osmesa', initialize an X connection
        // and let the default GL initialization happen.
        // For 'osmesa', libosmesa.so from the build needs to be in the same directory
        // as the hosting application.
        CommandLine* commandLine = CommandLine::ForCurrentProcess();
        gfx::GLImplementation requestedGLImplementation = gfx::kGLImplementationNone;
        if (commandLine->HasSwitch(switches::kUseGL)) {
            std::string requestedGLName = commandLine->GetSwitchValueASCII(switches::kUseGL);
            // Can't use this, so compare manually.
            //XXX http://code.google.com/p/chromium/issues/detail?id=67001
            //requestedGLImplementation = gfx::GetNamedGLImplementation(requestedGLName);
            if (requestedGLName == gfx::kGLImplementationOSMesaName)
                requestedGLImplementation = gfx::kGLImplementationOSMesaGL;
        }
        if (requestedGLImplementation == gfx::kGLImplementationOSMesaGL)
            gfx::InitializeGLBindings(gfx::kGLImplementationOSMesaGL);
        else {
            g_thread_init(NULL);
            gfx::GtkInitFromCommandLine(*commandLine);
        }
#endif

        WebKit::WebRuntimeFeatures::enableWebGL(true);

        WebKit::WebScriptController::enableV8SingleThreadMode();
        WebKit::WebScriptController::registerExtension(Chromix::ChromixExtension::Get());
    }

    ~MixKit() {
        // webkit_glue::ResourceLoaderBridge::Create() creates a SimpleResourceLoaderBridge
        // which spawns an IO thread - so need to shut it down.
        SimpleResourceLoaderBridge::Shutdown();
        WebKit::shutdown();
    }

private:
    // These setup global state - must be initialized before WebKitClient
    base::AtExitManager atExitManager;
    MessageLoop messageLoop;
    Chromix::WebKitClientImpl webKitClient;

    DISALLOW_COPY_AND_ASSIGN(MixKit);
};

static MixKit* mixKit = 0;

InitializeResult initialize(int argc, const char* argv[]) {
    InitializeResult result = InitializeAlready;
    if (!mixKit) {
        mixKit = new MixKit(argc, argv);
        result = mixKit ? InitializeSuccess : InitializeFailure;
    }
    return result;
}

void shutdown() {
    delete mixKit;
    mixKit = 0;
}

}
