// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <chromix/Chromix.h>
#include <chromix/MixRender.h>
extern "C" {
    #include <mlt/framework/mlt.h>
}
#include "chromix_service.h"

mlt_frame chromix_filter_process(mlt_filter self, mlt_frame frame);
int chromix_producer_get_frame(mlt_producer producer, mlt_frame_ptr frame, int index);
mlt_frame chromix_transition_process(mlt_transition transition, mlt_frame a_frame, mlt_frame b_frame);

#define YML_SUFFIX ".yml"


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

// Includes trailing slash
const std::string chromix_get_metadata_dir() {
    static const std::string metadata_dir(std::string(mlt_environment("MLT_DATA")).append("/chromix/"));
    return metadata_dir;
}

//XXX http://mltframework.org/twiki/bin/view/MLT/MetadataRequirements
static mlt_properties chromix_create_metadata(mlt_service_type service_type, const char* service_name, void*) {
    std::string metadata_path = chromix_get_metadata_dir() + service_name + YML_SUFFIX;
	return mlt_properties_parse_yaml(metadata_path.c_str());
}

static void* chromix_create_service(mlt_profile profile, mlt_service_type service_type, const char* service_name, const void*) {
    switch (service_type) {
        case producer_type: {
            mlt_producer self = mlt_producer_new();
            if (self) {
                self->get_frame = chromix_producer_get_frame;
                mlt_properties_set_data(MLT_PRODUCER_PROPERTIES(self), CHROMIX_METADATA_PROP,
                                        chromix_create_metadata(service_type, service_name, NULL),
                                        0, (mlt_destructor)mlt_properties_close, NULL);
                return self;
            }
            break;
        }
        case filter_type: {
            mlt_filter self = mlt_filter_new();
            if (self) {
                self->process = chromix_filter_process;
                mlt_properties_set_data(MLT_FILTER_PROPERTIES(self), CHROMIX_METADATA_PROP,
                                        chromix_create_metadata(service_type, service_name, NULL),
                                        0, (mlt_destructor)mlt_properties_close, NULL);
                return self;
            }
            break;
        }
        case transition_type: {
            mlt_transition self = mlt_transition_new();
            if (self) {
                self->process = chromix_transition_process;
                mlt_properties_set_data(MLT_TRANSITION_PROPERTIES(self), CHROMIX_METADATA_PROP,
                                        chromix_create_metadata(service_type, service_name, NULL),
                                        0, (mlt_destructor)mlt_properties_close, NULL);
                // Video only transition
                mlt_properties_set_int(MLT_TRANSITION_PROPERTIES(self), "_transition_type", 1);
                return self;
            }
            break;
        }
        default:
            break;
    }

    return NULL;
}

void chromix_register_services(mlt_repository repository, mlt_service_type service_type) {
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
