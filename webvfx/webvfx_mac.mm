#include <QTGlobal>

#ifdef Q_WS_MAC

#include <Foundation/NSThread.h>
#include "webvfx/webvfx.h"

namespace WebVFX {

bool isMainThread()
{
    return [NSThread isMainThread];
}

}


#endif
