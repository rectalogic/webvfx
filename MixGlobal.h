#ifndef MOTIONBOX_MIXKIT_MIXGLOBAL_H_
#define MOTIONBOX_MIXKIT_MIXGLOBAL_H_

#include <base/message_loop.h>
#include <base/at_exit.h>

#include "WebKitClientImpl.h"

namespace MixKit {

class MixGlobal {
public:
    MixGlobal();
    ~MixGlobal();

private:
    // These setup global state - must be initialized before WebKitClient
    base::AtExitManager atExitManager;
    MessageLoop messageLoop; //XXX test_shell uses MessageLoopForUI, but this might be OK
    MixKit::WebKitClientImpl webKitClient;
};

}
#endif

