#include "MixKit.h"

#include <public/WebKit.h>

// webKitClient depends on state initialized by messageLoop
MixKit::MixKit() : atExitManager(), messageLoop(), webKitClient() {
    WebKit::initialize(&webKitClient);
}

MixKit::~MixKit() {
    WebKit::shutdown();
}