// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFXPIPE_SERVICE_LOCKER_H_
#define VFXPIPE_SERVICE_LOCKER_H_

extern "C" {
    #include <framework/mlt_service.h>
}


namespace VFXPipe
{
class ServiceManager;

class ServiceLocker
{
public:
    ServiceLocker(mlt_service service);
    ~ServiceLocker();

    bool initialize(int width, int height, mlt_position length);
    ServiceManager* getManager();

private:
    static const char* kManagerPropertyName;
    static void destroyManager(ServiceManager* manager);
    mlt_service service;
    ServiceManager* manager;
};

}

#endif
