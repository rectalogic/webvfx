// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_PARAMETERMAP_H_
#define CHROMIX_PARAMETERMAP_H_

#include "chromix/ParameterValue.h"
#include <string>
#include <third_party/WebKit/WebCore/config.h>
#include <third_party/WebKit/WebCore/html/ImageData.h>
#include <third_party/WebKit/JavaScriptCore/wtf/RefPtr.h>
#include <base/hash_tables.h>

namespace v8 {
    class Value;
    template <class T> class Handle;
}

namespace Chromix {

class MixRender;

class ParameterMap {
public:
    ParameterMap();
    virtual ~ParameterMap() {};

    void setParameterValue(const std::string& name, WTF::PassRefPtr<ParameterValue> prpParam);

    // Returns buffer to write image data to - RGBA format, so size is width*height*4
    unsigned char* writeableDataForImageParameter(const std::string& name, unsigned int width, unsigned int height);

    v8::Handle<v8::Value> getImageParameterValue(const std::string& name) const;
    v8::Handle<v8::Value> getParameterValue(const std::string& name) const;

    //XXX APIs to enumerate (keys and descriptions)

private:
    typedef base::hash_map<std::string, ImageParameterValue> ImageParamMap;
    ImageParamMap imageParamMap;

    typedef base::hash_map<std::string, WTF::RefPtr<ParameterValue> > ParamMap;
    ParamMap paramMap;
};

}

#endif
