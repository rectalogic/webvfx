// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

extern "C" {
    #include <mlt/framework/mlt.h>
}

mlt_frame chromix_transition_process(mlt_transition transition, mlt_frame a_frame, mlt_frame b_frame) {
    //XXX
    //XXX need to chromix_initialize_service_properties from get_image
    return NULL;
}
