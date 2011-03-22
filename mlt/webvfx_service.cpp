// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <cstring>
#include <webvfx/webvfx.h>
#include "webvfx_service.h"
#include "service_manager.h"

#define YML_SUFFIX ".yml"
#define HTML_SUFFIX ".html"
#define QML_SUFFIX ".qml"


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

// See http://mltframework.org/twiki/bin/view/MLT/MetadataRequirements
static mlt_properties createMetadata(mlt_service_type serviceType, const char* serviceName, void*) {
    std::string metadataPath(getDataDir());
    metadataPath.append(serviceName).append(YML_SUFFIX);

    // File may not exists, metadata will be empty properties in that case
    mlt_properties metadata = mlt_properties_parse_yaml(metadataPath.c_str());
    mlt_properties_set(metadata, "identifier", serviceName);
    mlt_properties_set(metadata, "type", serviceTypeToName(serviceType));
    // tags array
    mlt_properties tags = mlt_properties_new();
    mlt_properties_set(tags, "0", "Video");
    mlt_properties_set_data(metadata, "tags", tags, 0, reinterpret_cast<mlt_destructor>(mlt_properties_close), 0);
    //XXX set extra params implied by images hash
    return metadata;
}


static void* createService(mlt_profile profile, mlt_service_type serviceType, const char* serviceName, const void*) {
    if (!WebVfx::initialize())
        return 0;

    mlt_service service = 0;
    switch (serviceType) {
        case producer_type:
            service = MLTWebVfx::createProducer(profile);
            break;
        case filter_type:
            service = MLTWebVfx::createFilter();
            break;
        case transition_type:
            service = MLTWebVfx::createTransition();
            break;
        default:
            break;
    }

    // Store filename property on service
    if (service) {
        std::string fileName(getDataDir());
        fileName.append(serviceName);
        mlt_properties_set(MLT_SERVICE_PROPERTIES(service), MLTWebVfx::ServiceManager::kFilePropertyName, fileName.c_str());
    }

    return service;
}

static void registerServices(mlt_repository repository, mlt_service_type serviceType, const std::string& wildcard, const std::string& dataDir)
{
    mlt_properties entries = mlt_properties_new();
    mlt_properties_dir_list(entries, dataDir.c_str(), wildcard.c_str(), 1);
    int entryCount = mlt_properties_count(entries);
    for (int i = 0; i < entryCount; i++) {
        char* path = mlt_properties_get_value(entries, i);
        const char* id = &path[dataDir.length() + 1];
        MLT_REGISTER(serviceType, id, createService);
        MLT_REGISTER_METADATA(serviceType, id, createMetadata, NULL);
    }
    mlt_properties_close(entries);
}

void MLTWebVfx::registerServices(mlt_repository repository, mlt_service_type serviceType) {
    // For service ID "webvfx.<service_type>.<service_name>.<suffix>"
    // we may have an optional "webvfx.<service_type>.<service_name>.<suffix>.yml" metadata.
    // Where "service_type" is filter, transition or producer.
    // And "suffix" is ".html" or ".qml"

    const char* serviceTypeName = serviceTypeToName(serviceType);
    if (!serviceTypeName)
        return;

    std::string dataDir(getDataDir());

    std::string serviceTypeWildcard("webvfx.");
    serviceTypeWildcard.append(serviceTypeName).append(".*");

    registerServices(repository, serviceType, serviceTypeWildcard + HTML_SUFFIX, dataDir);
    registerServices(repository, serviceType, serviceTypeWildcard + QML_SUFFIX, dataDir);
}
