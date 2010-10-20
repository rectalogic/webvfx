#include <pthread.h>
#include <chromix/MixKit.h>
extern "C" {
    #include <mlt/framework/mlt_factory.h>
}
#include "chromix_helper.h"

static Chromix::MixKit* mixKit = 0;
static pthread_mutex_t mixKitLock = PTHREAD_MUTEX_INITIALIZER;


static void chromix_shutdown(void *) {
    pthread_mutex_lock(&mixKitLock);
    delete mixKit;
    mixKit = 0;
    pthread_mutex_unlock(&mixKitLock);
}

//XXX each filter/trans etc. must call this from their init
static bool chromix_init() {
    pthread_mutex_lock(&mixKitLock);
    if (!mixKit) {
        mixKit = new Chromix::MixKit(0, NULL);
        mlt_factory_register_for_clean_up(0, chromix_shutdown);
    }
    pthread_mutex_unlock(&mixKitLock);
    return mixkit;
}

extern "C" int chromix_initialize(mlt_properties prop) {
    if (!chromix_init())
        return 0;
    //XXX need to delete MixRender instance - or can we just set it as a property with property destructor via mlt_properties_set_data?
    //XXX add C++ helper file - manage creating/storing MixRender with destructor, setting properties we get from it, dealing with getting the writeable image etc.
    //XXX also put chromix_init there - then the impl files can be C
    
}
