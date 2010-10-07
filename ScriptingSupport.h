#ifndef MOTIONBOX_CHROMIX_SCRIPTINGSUPPORT_H_
#define MOTIONBOX_CHROMIX_SCRIPTINGSUPPORT_H_

#include "ParameterMap.h"

namespace Chromix {

class ScriptingSupport {
public:
    ScriptingSupport();
    virtual ~ScriptingSupport();

    ParameterMap& getParameterMap() { return parameterMap; }

private:
    ParameterMap parameterMap;
};

}

#endif
