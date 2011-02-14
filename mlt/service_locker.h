// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MLTWEBVFX_SERVICE_LOCKER_H_
#define MLTWEBVFX_SERVICE_LOCKER_H_

extern "C" {
    #include <mlt/framework/mlt_service.h>
}


namespace MLTWebVfx
{
class ServiceManager;

class ServiceLocker
{
public:
    ServiceLocker(mlt_service service);
    ~ServiceLocker();

    bool initialize(int width, int height);
    ServiceManager* getManager();

private:
    static const char* kManagerPropertyName;
    static void destroyManager(ServiceManager* manager);
    mlt_service service;
    ServiceManager* manager;
};

}

#endif
