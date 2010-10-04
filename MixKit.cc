#include "MixKit.h"
#include "ImageExtension.h"

#include <base/command_line.h>
#include <third_party/WebKit/WebKit/chromium/public/WebKit.h>
#include <third_party/WebKit/WebKit/chromium/public/WebRuntimeFeatures.h>
#include <third_party/WebKit/WebKit/chromium/public/WebScriptController.h>
#if defined(OS_MACOSX)
#include <third_party/WebKit/WebKit/mac/WebCoreSupport/WebSystemInterface.h>
#endif

// webKitClient depends on state initialized by messageLoop
Chromix::MixKit::MixKit(int argc, const char* argv[]) : atExitManager(), messageLoop(), webKitClient() {
    WebKit::initialize(&webKitClient);
    CommandLine::Init(argc, argv);
#if defined(OS_MACOSX)
    InitWebCoreSystemInterface();
#endif
    WebKit::WebRuntimeFeatures::enableWebGL(true);

    WebKit::WebScriptController::enableV8SingleThreadMode();
    WebKit::WebScriptController::registerExtension(Chromix::ImageExtensionV8::Get());
}

Chromix::MixKit::~MixKit() {
    WebKit::shutdown();
}
