#include "MixKit.h"
#include "MixKitPrivate.h"
#include "ChromixExtension.h"

#include <base/command_line.h>
#include <app/gfx/gl/gl_implementation.h>
#include <third_party/WebKit/WebKit/chromium/public/WebKit.h>
#include <third_party/WebKit/WebKit/chromium/public/WebRuntimeFeatures.h>
#include <third_party/WebKit/WebKit/chromium/public/WebScriptController.h>
#if defined(OS_MACOSX)
#include <third_party/WebKit/WebKit/mac/WebCoreSupport/WebSystemInterface.h>
#endif


Chromix::MixKit::MixKit(int argc, const char* argv[]) : impl(new MixKitPrivate(argc, argv))
{
}

Chromix::MixKit::~MixKit() {
    delete impl;
}


// webKitClient depends on state initialized by messageLoop
Chromix::MixKitPrivate::MixKitPrivate(int argc, const char* argv[]) :
    atExitManager(),
    messageLoop(),
    webKitClient()
{
    CommandLine::Init(argc, argv);

    WebKit::initialize(&webKitClient);
#if defined(OS_MACOSX)
    InitWebCoreSystemInterface();
#endif
    gfx::InitializeGLBindings(gfx::kGLImplementationOSMesaGL);
    WebKit::WebRuntimeFeatures::enableWebGL(true);

    WebKit::WebScriptController::enableV8SingleThreadMode();
    WebKit::WebScriptController::registerExtension(Chromix::ChromixExtension::Get());
}

Chromix::MixKitPrivate::~MixKitPrivate() {
    WebKit::shutdown();
}
