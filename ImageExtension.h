#ifndef MOTIONBOX_CHROMIX_IMAGEEXTENSION_H_
#define MOTIONBOX_CHROMIX_IMAGEEXTENSION_H_

#include <v8/include/v8.h>

namespace Chromix {

class ImageExtensionV8 {
public:
    static v8::Extension* Get();
};

}
#endif
