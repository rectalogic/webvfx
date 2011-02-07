#include <QTGlobal>

#ifdef Q_WS_MAC

#include <Foundation/NSThread.h>
#include "webfx/webfx.h"

namespace WebFX {

bool isMainThread()
{
    return [NSThread isMainThread];
}

}


#endif
