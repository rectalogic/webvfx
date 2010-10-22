#include <string>
#include <chromix/Chromix.h>
#include <chromix/MixRender.h>
extern "C" {
    #include <mlt/framework/mlt_factory.h>
    #include <mlt/framework/mlt_properties.h>
}
#include "chromix_helper.h"

//XXX static const char* MIXRENDER_PROP = "MixRender";
#define YML_SUFFIX ".yml"

/*XXX
static void chromix_shutdown(void*) {
    Chromix::shutdown();
}

static void chromix_delete_mixrender(Chromix::MixRender* mixRender) {
    delete mixRender;
}

static bool chromix_initialize() {
    Chromix::InitializeResult result = Chromix::initialize();
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
*/

static const char* service_type_to_name(mlt_service_type service_type) {
    switch (service_type) {
        case producer_type:
            return "producer";
        case filter_type:
            return "filter";
        case transition_type:
            return "transition";
        default:
            return NULL;
    }
}

static const std::string& chromix_get_metadata_dir() {
    static std::string chromix_metadata_dir = std::string(mlt_environment("MLT_DATA")).append("/chromix");
    return chromix_metadata_dir;
}

//XXX http://mltframework.org/twiki/bin/view/MLT/MetadataRequirements
static mlt_properties chromix_create_metadata(mlt_service_type service_type, const char* service_name, void*) {
    const char* service_type_name = service_type_to_name(service_type);
    if (!service_type_name)
        return NULL;

    std::string metadata_yml = chromix_get_metadata_dir() + service_name + YML_SUFFIX;
	return mlt_properties_parse_yaml(metadata_yml.c_str());
}

static void chromix_create_service(mlt_profile, mlt_service_type, const char * /* service name */, const void * /* arg */ ) {
    //XXX
}

extern "C" void chromix_register_services(mlt_repository repository, mlt_service_type service_type) {
    // Metadata is named "chromix.<service_name>.filter.yml" and the ID is "chromix.<service_name>.filter"
    // This is so we can have e.g. both a filter and transition with the same <service_name> name.

    const char* service_type_name = service_type_to_name(service_type);
    if (!service_type_name)
        return;

    std::string chromix_metadata_dir(chromix_get_metadata_dir());

    // "chromix.*.<service_type_name>.yml" e.g. "chromix.*.filter.yml"
    std::string service_type_wildcard("chromix.*.");
    service_type_wildcard.append(service_type_name).append(YML_SUFFIX);

    mlt_properties metadata_entries = mlt_properties_new();
    mlt_properties_dir_list(metadata_entries, chromix_metadata_dir.c_str(), service_type_wildcard.c_str(), 1);
    int metadata_count = mlt_properties_count(metadata_entries);
    for (int i = 0; i < metadata_count; i++) {
        char* pathname = mlt_properties_get_value(metadata_entries, i);

        // ID is "chromix.<service_name>.<service_type_name>" e.g. "chromix.bubbles.filter"
        const char* id_start = &pathname[chromix_metadata_dir.length() + 1];
        int id_length = strlen(id_start) + sizeof(YML_SUFFIX) - 1;
        std::string service_id(id_start, id_length);

        const char* id = service_id.c_str();
        MLT_REGISTER(service_type, id, chromix_create_service);
        MLT_REGISTER_METADATA(service_type, id, chromix_create_metadata, NULL);
    }
    mlt_properties_close(metadata_entries);
}
