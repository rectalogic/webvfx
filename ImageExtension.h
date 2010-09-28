#ifndef MOTIONBOX_MIXKIT_IMAGEEXTENSION_H_
#define MOTIONBOX_MIXKIT_IMAGEEXTENSION_H_

#include <v8/include/v8.h>

namespace MixKit {

class ImageExtensionV8 {
public:
    static v8::Extension* Get();
};

}
#endif