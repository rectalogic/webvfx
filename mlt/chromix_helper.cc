#include <string>
#include <chromix/Chromix.h>
#include <chromix/MixRender.h>
extern "C" {
    #include <mlt/framework/mlt.h>
}
#include "chromix_helper.h"

static const char* MIXRENDER_PROP = "MixRender";
#define YML_SUFFIX ".yml"
#define FAILURE 1
#define SUCCESS 0

static void chromix_shutdown(void*) {
    Chromix::shutdown();
}

static bool chromix_initialize() {
    Chromix::InitializeResult result = Chromix::initialize();
    if (result == Chromix::InitializeSuccess)
        mlt_factory_register_for_clean_up(NULL, chromix_shutdown);
    return result != Chromix::InitializeFailure;
}

static void chromix_delete_mixrender(Chromix::MixRender* mixRender) {
    delete mixRender;
}

static int chromix_initialize_service_properties(mlt_properties properties) {
    if (!chromix_initialize())
        return FAILURE;

    // Create and stash renderer on properties
    Chromix::MixRender* mixRender = new Chromix::MixRender();
    if (!mixRender)
        return FAILURE;
    mlt_properties_set_data(properties, MIXRENDER_PROP, mixRender, 0, (mlt_destructor)chromix_delete_mixrender, NULL);

    //XXX need to set mixRender properties from mlt_properties_get, iterate over metadata
    //XXX should chromix use a callback to get properties from the app? would need to map to v8 datatype
    mixRender->loadURL(WTF::String::fromUTF8("file://localhost/Users/aw/Projects/snapfish/encoder/chromix/test/test.html"));//XXX
    //XXX get and load html page property here, if not specified assume same as service_name".html"

    //XXX for a_track/b_track for transition, "track" for filter, and other tracks
    //XXX add these in yaml as custom props - mapping track to prop name used in html?
    //XXX store this mapping on properties - map char* well defined name (for filter, trans) to WTF::String we can use w/chromix

    //XXX add method dealing with getting the writeable image etc.

    //XXX setup logging using mlt_log - should we map chrome levels to mlt? are crhome levels accurate?

    return SUCCESS;
}

//XXX lookup track property to get the WTF::String mixrender name for the image
int chromix_set_image(mlt_properties properties, const char* track, uint8_t* image, int width, int height) {
    Chromix::MixRender* mixRender = (Chromix::MixRender*)mlt_properties_get_data(properties, MIXRENDER_PROP, NULL);
    if (!mixRender)
        return FAILURE;
    //XXX lookup param - map track to WTF::String
    unsigned char* buffer = mixRender->writeableDataForImageParameter(WTF::String::fromUTF8("video"), width, height);
    if (!buffer)
        return FAILURE;
    memcpy(buffer, image, width * height * 4);
    return SUCCESS;
}

int chromix_render(mlt_properties properties, double time, uint8_t* image, int width, int height) {
    Chromix::MixRender* mixRender = (Chromix::MixRender*)mlt_properties_get_data(properties, MIXRENDER_PROP, NULL);
    if (!mixRender)
        return FAILURE;
    mixRender->resize(width, height);
    const SkBitmap* skiaBitmap = mixRender->render(time);
    if (!skiaBitmap)
        return FAILURE;
    SkAutoLockPixels bitmapLock(*skiaBitmap);
    // Do a block copy if no padding, otherwise copy a row at a time
    unsigned int byteCount = width * height * 4;
    if (skiaBitmap->getSize() == byteCount)
        memcpy(image, skiaBitmap->getPixels(), byteCount);
    else {
        int bytesPerRow = width * 4;
        const unsigned char* srcP = reinterpret_cast<const unsigned char*>(skiaBitmap->getPixels());
        unsigned char* dstP = image;
        for (int y = 0; y < height; y++) {
            memcpy(dstP, srcP, bytesPerRow);
            srcP += skiaBitmap->rowBytes();
            dstP += bytesPerRow;
        }
    }
    return SUCCESS;
}

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

static const std::string chromix_get_metadata_dir() {
    return std::string(mlt_environment("MLT_DATA")).append("/chromix");
}

//XXX http://mltframework.org/twiki/bin/view/MLT/MetadataRequirements
static mlt_properties chromix_create_metadata(mlt_service_type service_type, const char* service_name, void*) {
    const char* service_type_name = service_type_to_name(service_type);
    if (!service_type_name)
        return NULL;

    std::string metadata_yml = chromix_get_metadata_dir() + service_name + YML_SUFFIX;
	return mlt_properties_parse_yaml(metadata_yml.c_str());
}

static void* chromix_create_service(mlt_profile profile, mlt_service_type service_type, const char* service_name, const void*) {
    void* service = NULL;
    switch (service_type) {
        case producer_type: {
            mlt_producer self = mlt_producer_new();
            if (self) {
                self->get_frame = chromix_producer_get_frame;
                service = self;
                if (chromix_initialize_service_properties(MLT_PRODUCER_PROPERTIES(self)) != 0) {
                    mlt_producer_close(self);
                    return NULL;
                }
            }
            break;
        }
        case filter_type: {
            mlt_filter self = mlt_filter_new();
            if (self) {
                self->process = chromix_filter_process;
                service = self;
                if (chromix_initialize_service_properties(MLT_FILTER_PROPERTIES(self)) != 0) {
                    mlt_filter_close(self);
                    return NULL;
                }
            }
            break;
        }
        case transition_type: {
            mlt_transition self = mlt_transition_new();
            if (self) {
                self->process = chromix_transition_process;
                service = self;
                if (chromix_initialize_service_properties(MLT_TRANSITION_PROPERTIES(self)) != 0) {
                    mlt_transition_close(self);
                    return NULL;
                }
            }
            break;
        }
        default:
            return NULL;
    }
    
    return service;
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
        int id_length = strlen(id_start) - (sizeof(YML_SUFFIX) - 1);
        std::string service_id(id_start, id_length);

        const char* id = service_id.c_str();
        MLT_REGISTER(service_type, id, chromix_create_service);
        MLT_REGISTER_METADATA(service_type, id, chromix_create_metadata, NULL);
    }
    mlt_properties_close(metadata_entries);
}
