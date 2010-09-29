#import <Foundation/Foundation.h>

#include <base/basictypes.h>
#include <base/message_loop.h>

#include <gfx/codec/png_codec.h>

#include "MixGlobal.h"
#include "MixRender.h"

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    MixKit::MixGlobal mixKit;
    MixKit::MixRender mixRender(400, 300);

    mixRender.loadURL("file://localhost/Users/aw/Projects/snapfish/encoder/MixKit/test.html");
    const SkBitmap &skiaBitmap = mixRender.render();

    // Encode pixel data to PNG.
    std::vector<unsigned char> pngData;
    SkAutoLockPixels bitmapLock(skiaBitmap);
    gfx::PNGCodec::Encode(reinterpret_cast<const unsigned char*>(skiaBitmap.getPixels()),
                          gfx::PNGCodec::FORMAT_BGRA, skiaBitmap.width(), skiaBitmap.height(),
                          static_cast<int>(skiaBitmap.rowBytes()), false, &pngData);

    // Write to disk. Cheat and be MacOS specific since this is just for testing.
    NSData *data = [NSData dataWithBytesNoCopy:reinterpret_cast<void *>(&pngData[0])
                                        length:pngData.size()
                                  freeWhenDone:NO];
    [data writeToFile:@"/tmp/render.png" atomically:NO];

    [pool drain];
    return 0;
}
