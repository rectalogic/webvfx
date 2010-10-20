#include <chromix/Chromix.h>
extern "C" {
    #include <mlt/framework/mlt_factory.h>
}
#include "chromix_helper.h"


static void chromix_shutdown(void *) {
    Chromix::shutdown();
}

static bool chromix_initialize() {
    return Chromix::initialize();
}

extern "C" int chromix_initialize_properties(mlt_properties prop) {
    if (!chromix_initialize())
        return 0;
    //XXX need to delete MixRender instance - or can we just set it as a property with property destructor via mlt_properties_set_data?
    //XXX add C++ helper file - manage creating/storing MixRender with destructor, setting properties we get from it, dealing with getting the writeable image etc.
    //XXX also put chromix_init there - then the impl files can be C
    
}
