#include <QtGlobal>

#ifdef Q_WS_MAC

#include <Foundation/NSThread.h>
#include "webvfx/webvfx.h"

namespace WebVfx {

bool isMainThread()
{
    return [NSThread isMainThread];
}

}


#endif
