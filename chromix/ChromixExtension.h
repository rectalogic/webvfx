#ifndef MOTIONBOX_CHROMIX_IMAGEEXTENSION_H_
#define MOTIONBOX_CHROMIX_IMAGEEXTENSION_H_

namespace v8 {
    class Extension;
}

namespace Chromix {

class ChromixExtension {
public:
    static v8::Extension* Get();
};

}
#endif
