#include "MixManager.h"

#include <public/WebKit.h>
#if defined(OS_MACOSX)
#include <third_party/WebKit/WebKit/mac/WebCoreSupport/WebSystemInterface.h>
#endif

// webKitClient depends on state initialized by messageLoop
MixKit::MixManager::MixManager() : atExitManager(), messageLoop(), webKitClient() {
    WebKit::initialize(&webKitClient);
#if defined(OS_MACOSX)
    InitWebCoreSystemInterface();
#endif
}

MixKit::MixManager::~MixManager() {
    WebKit::shutdown();
}