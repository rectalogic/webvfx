// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include "webvfx_service.h"

#define YML_SUFFIX ".yml"
#define HTML_SUFFIX ".html"


static const char* serviceTypeToName(mlt_service_type service_type) {
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
static const std::string getDataDir() {
    static const std::string dataDir(std::string(mlt_environment("MLT_DATA")).append("/webvfx/"));
    return dataDir;
}

//XXX http://mltframework.org/twiki/bin/view/MLT/MetadataRequirements
static mlt_properties createMetadata(mlt_service_type serviceType, const char* serviceName, void*) {
    std::string metadataPath(getDataDir());
    metadataPath.append(serviceName).append(YML_SUFFIX);

    // File may not exists, metadata will be empty properties in that case
	mlt_properties metadata = mlt_properties_parse_yaml(metadataPath.c_str());
    mlt_properties_set(metadata, "identifier", serviceName);
    mlt_properties_set(metadata, "type", serviceTypeToName(serviceType));
    //XXX set extra params implied by images hash
    return metadata;
}


static void* createService(mlt_profile profile, mlt_service_type serviceType, const char* serviceName, const void*) {
    if (!WebVFX::initialize())
        return 0;

    mlt_service service = 0;
    switch (service_type) {
        case producer_type:
            service = MLTWebVFX::createProducer(serviceName);
            break;
        case filter_type:
            service = MLTWebVFX::createFilter(serviceName);
            break;
        case transition_type:
            service = MLTWebVFX::createTransition(serviceName);
            break;
        default:
            break;
    }

    if (service) {
        std::string url("file://");
        url.append(getDataDir()).append(serviceName).append(HTML_SUFFIX);
        mlt_properties_set(MLT_SERVICE_PROPERTIES(service), ServiceManager::kURLPropertyName, url.c_str());
    }

    return service;
}

void MLTWebVFX::registerServices(mlt_repository repository, mlt_service_type serviceType) {
    // For service ID "webvfx.<service_type>.<service_name>", we have "webvfx.<service_type>.<service_name>.html"
    // and optional "webvfx.<service_type>.<service_name>.yml" metadata.
    // Where "service_type" is filter, transition or producer.

    const char* serviceTypeName = serviceTypeToName(serviceType);
    if (!serviceTypeName)
        return;

    std::string dataDir(getDataDir());

    // "webvfx.<service_type>.*.html" e.g. "webvfx.filter.*.html"
    std::string serviceTypeWildcard("webvfx.");
    serviceTypeWildcard.append(serviceTypeName).append(".*" HTML_SUFFIX);

    mlt_properties htmlEntries = mlt_properties_new();
    mlt_properties_dir_list(metadataEntries, dataDir.c_str(), serviceTypeWildcard.c_str(), 1);
    int htmlCount = mlt_properties_count(metadataEntries);
    for (int i = 0; i < htmlCount; i++) {
        char* htmlPath = mlt_properties_get_value(metadataEntries, i);

        // ID is "webvfx.<service_type>.<service_name>" e.g. "webvfx.filter.bubbles"
        const char* idStart = &pathname[dataDir.length() + 1];
        int idLength = strlen(idStart) - (sizeof(HTML_SUFFIX) - 1);
        std::string serviceID(idStart, idLength);

        const char* id = serviceID.c_str();
        MLT_REGISTER(serviceType, id, createService);
        MLT_REGISTER_METADATA(serviceType, id, createMetadata, NULL);
    }
    mlt_properties_close(metadataEntries);
}
