#include "MixManager.h"
#include "ImageExtension.h"

#include <third_party/WebKit/WebKit/chromium/public/WebKit.h>
#include <third_party/WebKit/WebKit/chromium/public/WebScriptController.h>
#if defined(OS_MACOSX)
#include <third_party/WebKit/WebKit/mac/WebCoreSupport/WebSystemInterface.h>
#endif

// webKitClient depends on state initialized by messageLoop
MixKit::MixManager::MixManager() : atExitManager(), messageLoop(), webKitClient() {
    WebKit::initialize(&webKitClient);
#if defined(OS_MACOSX)
    InitWebCoreSystemInterface();
#endif
    //XXX WebKit::WebRuntimeFeatures::enableWebGL(true);

    WebKit::WebScriptController::registerExtension(MixKit::ImageExtensionV8::Get());
}

MixKit::MixManager::~MixManager() {
    WebKit::shutdown();
}
