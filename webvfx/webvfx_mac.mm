#include <QtGlobal>

#ifdef Q_OS_MAC

#include <Foundation/NSThread.h>
#include "webvfx/webvfx.h"

namespace WebVfx {

bool isMainThread()
{
    return [NSThread isMainThread];
}

}


#endif
