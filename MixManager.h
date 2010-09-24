#ifndef MOTIONBOX_MIXKIT_MIXMANAGER_H_
#define MOTIONBOX_MIXKIT_MIXMANAGER_H_

#include <base/message_loop.h>
#include <base/at_exit.h>

#include "WebKitClientImpl.h"

namespace MixKit {

class MixManager {
public:
    MixManager();
    ~MixManager();

private:
    // These setup global state - must be initialized before WebKitClient
    base::AtExitManager atExitManager;
    MessageLoop messageLoop; //XXX test_shell uses MessageLoopForUI, but this might be OK
    MixKit::WebKitClientImpl webKitClient;
};

}
#endif