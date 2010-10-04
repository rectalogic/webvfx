#ifndef MOTIONBOX_CHROMIX_MIXKIT_H_
#define MOTIONBOX_CHROMIX_MIXKIT_H_

#include <base/message_loop.h>
#include <base/at_exit.h>

#include "WebKitClientImpl.h"

namespace Chromix {

class MixKit {
public:
    MixKit(int argc, const char* argv[]);
    ~MixKit();

private:
    // These setup global state - must be initialized before WebKitClient
    base::AtExitManager atExitManager;
    MessageLoop messageLoop; //XXX test_shell uses MessageLoopForUI, but this might be OK
    Chromix::WebKitClientImpl webKitClient;
};

}
#endif

