#ifndef MOTIONBOX_CHROMIX_MIXKITPRIVATE_H_
#define MOTIONBOX_CHROMIX_MIXKITPRIVATE_H_

#include "WebKitClientImpl.h"

#include <base/message_loop.h>
#include <base/at_exit.h>

namespace Chromix {
    
    class MixKitPrivate {
    public:
        MixKitPrivate(int argc, const char* argv[]);
        ~MixKitPrivate();
        
    private:
        // These setup global state - must be initialized before WebKitClient
        base::AtExitManager atExitManager;
        MessageLoop messageLoop;
        Chromix::WebKitClientImpl webKitClient;
    };
    
}
#endif
