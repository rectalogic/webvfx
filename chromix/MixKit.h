#ifndef MOTIONBOX_CHROMIX_MIXKIT_H_
#define MOTIONBOX_CHROMIX_MIXKIT_H_

#include <base/basictypes.h>

namespace Chromix {

class MixKitPrivate;

//XXX need to ensure only one MixKit is created
class MixKit {
public:
    MixKit(int argc, const char* argv[]);
    ~MixKit();
private:
    MixKitPrivate *impl;

    DISALLOW_COPY_AND_ASSIGN(MixKit);
};

}
#endif

