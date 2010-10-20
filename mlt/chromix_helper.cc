#include <chromix/Chromix.h>
#include <chromix/MixRender.h>
extern "C" {
    #include <mlt/framework/mlt_factory.h>
}
#include "chromix_helper.h"

static const char* MIXRENDER_PROP = "MixRender";

static void chromix_shutdown(void*) {
    Chromix::shutdown();
}

static void chromix_delete_mixrender(Chromix::MixRender* mixRender) {
    delete mixRender;
}

static bool chromix_initialize() {
    Chromix::Initialize result = Chromix::initialize();
    if (result == Chromix::InitializeSuccess)
        mlt_factory_register_for_clean_up(NULL, chromix_shutdown);
    return result != Chromix::InitializeFailure;
}

extern "C" int chromix_initialize_properties(mlt_properties properties) {
    if (!chromix_initialize())
        return 0;

    // Create and stash renderer on properties
    Chromix::MixRender* mixRender = new Chromix::MixRender();
    if (!mixRender)
        return 0;
    mlt_properties_set_data(properties, MIXRENDER_PROP, mixRender, 0, (mlt_destructor)chromix_delete_mixrender, NULL);

    //XXX  set properties supported by the html via MLT_REGISTER_METADATA on a metadata props
    //XXX but then have to load all effects in factory (see frei0r) - can we use yaml to describe ourself (also allows us to specify if we are filter/producer etc. - see oldfilm)
    //XXX oldfilm does MLT_REGISTER_METADATA with mlt_properties_parse_yaml
    
    //XXX change Chromix so page doesn't register params, but app should register params with values that page can fetch
    //XXX so app must configure itself externally (yaml in this case) with params/values it knows the page needs
    //XXX can also then support other datatypes?
    //XXX what about a_track/b_track for transition, "track" for filter, and other tracks?
    //XXX add these in yaml as custom props - mapping track to name used in html?

    //XXX add method dealing with getting the writeable image etc.

    //XXX setup logging using mlt_log - should we map chrome levels to mlt? are crhome levels accurate?

    return 1;
}
