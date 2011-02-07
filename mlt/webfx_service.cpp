// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "webfx_service.h"

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
const std::string webfx_get_metadata_dir() {
    static const std::string metadata_dir(std::string(mlt_environment("MLT_DATA")).append("/webfx/"));
    return metadata_dir;
}

mlt_properties chromix_load_metadata(const std::string& service_name) {
    std::string metadata_path = webfx_get_metadata_dir() + service_name + YML_SUFFIX;
	return mlt_properties_parse_yaml(metadata_path.c_str());
}

//XXX http://mltframework.org/twiki/bin/view/MLT/MetadataRequirements
static mlt_properties chromix_create_metadata(mlt_service_type service_type, const char* service_name, void*) {
    mlt_properties metadata = chromix_load_metadata(service_name);
    mlt_properties_set(metadata, "identifier", service_name);
    mlt_properties_set(metadata, "type", service_type_to_name(service_type));
    //XXX set extra params implied by images hash
    return metadata;
}

static void* webfx_service_create(mlt_profile profile, mlt_service_type service_type, const char* service_name, const void*) {
    if (!WebFX::initialize())
        return NULL;

    switch (service_type) {
        case producer_type:
            return webfx_producer_create(service_name);
        case filter_type:
            return webfx_filter_create(service_name);
        case transition_type:
            return webfx_transition_create(service_name);
        default:
            break;
    }

    return NULL;
}

void webfx_register_services(mlt_repository repository, mlt_service_type service_type) {
    // Metadata is named e.g. "webfx.filter.<service_name>.yml" and the ID is "webfx.filter.<service_name>"
    // This is so we can have e.g. both a filter and transition with the same <service_name> name.

    const char* service_type_name = service_type_to_name(service_type);
    if (!service_type_name)
        return;

    std::string webfx_metadata_dir(webfx_get_metadata_dir());

    // "webfx.<service_type_name>.*.yml" e.g. "webfx.filter.*.yml"
    std::string service_type_wildcard("webfx.");
    service_type_wildcard.append(service_type_name).append(".*" YML_SUFFIX);

    mlt_properties metadata_entries = mlt_properties_new();
    mlt_properties_dir_list(metadata_entries, webfx_metadata_dir.c_str(), service_type_wildcard.c_str(), 1);
    int metadata_count = mlt_properties_count(metadata_entries);
    for (int i = 0; i < metadata_count; i++) {
        char* pathname = mlt_properties_get_value(metadata_entries, i);

        // ID is "webfx.<service_type_name>.<service_name>" e.g. "webfx.filter.bubbles"
        const char* id_start = &pathname[webfx_metadata_dir.length() + 1];
        int id_length = strlen(id_start) - (sizeof(YML_SUFFIX) - 1);
        std::string service_id(id_start, id_length);

        const char* id = service_id.c_str();
        MLT_REGISTER(service_type, id, webfx_service_create);
        MLT_REGISTER_METADATA(service_type, id, webfx_create_metadata, NULL);
    }
    mlt_properties_close(metadata_entries);
}
