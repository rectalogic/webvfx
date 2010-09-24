#ifndef MOTIONBOX_MIXKIT_H_
#define MOTIONBOX_MIXKIT_H_

#include <base/message_loop.h>
#include <base/at_exit.h>

#include "MixKitWebKitClient.h"

class MixKit {
public:
    MixKit();
    ~MixKit();

private:
    // These setup global state - must be initialized before WebKitClient
    base::AtExitManager atExitManager;
    MessageLoop messageLoop; //XXX test_shell uses MessageLoopForUI, but this might be OK
    MixKitWebKitClient webKitClient;
};

#endif