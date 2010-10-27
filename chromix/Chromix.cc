#include "chromix/Chromix.h"
#include "chromix/ChromixExtension.h"
#include "chromix/WebKitClientImpl.h"

#include <base/message_loop.h>
#include <base/at_exit.h>
#include <base/command_line.h>
#include <app/gfx/gl/gl_implementation.h>
#include <third_party/WebKit/WebKit/chromium/public/WebKit.h>
#include <third_party/WebKit/WebKit/chromium/public/WebRuntimeFeatures.h>
#include <third_party/WebKit/WebKit/chromium/public/WebScriptController.h>
#if defined(OS_MACOSX)
#include <third_party/WebKit/WebKit/mac/WebCoreSupport/WebSystemInterface.h>
#endif

namespace Chromix {

//XXX need to ensure only one MixKit is created
//XXX add Chromix::initialize/shutdown functions and create Singleton<MixKit>
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
#endif
        gfx::InitializeGLBindings(gfx::kGLImplementationOSMesaGL);
        WebKit::WebRuntimeFeatures::enableWebGL(true);
        
        WebKit::WebScriptController::enableV8SingleThreadMode();
        WebKit::WebScriptController::registerExtension(Chromix::ChromixExtension::Get());
    }

    ~MixKit() {
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

InitializeResult initialize() {
    InitializeResult result = InitializeAlready;
    if (!mixKit) {
        mixKit = new MixKit(0, NULL);
        result = mixKit ? InitializeSuccess : InitializeFailure;
    }
    return result;
}

void shutdown() {
    delete mixKit;
    mixKit = 0;
}

}
