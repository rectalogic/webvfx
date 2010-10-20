#include "chromix/Chromix.h"
#include "chromix/ChromixExtension.h"
#include "chromix/WebKitClientImpl.h"

#include <pthread.h>

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

static pthread_mutex_t chromixLock = PTHREAD_MUTEX_INITIALIZER;

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

bool initialize() {
    pthread_mutex_lock(&chromixLock);
    if (!mixKit)
        mixKit = new MixKit(0, NULL);
    bool result = mixKit;
    pthread_mutex_unlock(&chromixLock);
    return result;
}

void shutdown() {
    pthread_mutex_lock(&chromixLock);
    delete mixKit;
    mixKit = 0;
    pthread_mutex_unlock(&chromixLock);
}

}
