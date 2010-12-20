// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt.h>
}

int chromix_producer_get_frame(mlt_producer producer, mlt_frame_ptr frame, int index) {
    //XXX
    //XXX need to chromix_initialize_service_properties from get_image
    return 1;
}
