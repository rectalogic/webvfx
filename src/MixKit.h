#ifndef MOTIONBOX_CHROMIX_MIXKIT_H_
#define MOTIONBOX_CHROMIX_MIXKIT_H_

namespace Chromix {

class MixKitPrivate;

class MixKit {
public:
    MixKit(int argc, const char* argv[]);
    ~MixKit();

private:
    MixKitPrivate *impl;
};

}
#endif

