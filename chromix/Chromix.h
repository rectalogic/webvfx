#ifndef MOTIONBOX_CHROMIX_CHROMIX_H_
#define MOTIONBOX_CHROMIX_CHROMIX_H_

namespace Chromix {
enum InitializeResult { InitializeFailure, InitializeSuccess, InitializeAlready };
InitializeResult initialize();
void shutdown();    
}
#endif
