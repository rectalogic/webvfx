#import <Foundation/Foundation.h>

#include <iostream>
#include <fstream>

#include <base/basictypes.h>
#include <base/message_loop.h>
#include <gfx/codec/png_codec.h>

#include "MixKit.h"
#include "MixRender.h"

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    Chromix::MixKit mixKit;
    Chromix::MixRender mixRender(400, 300);

    mixRender.loadURL("file://localhost/Users/aw/Projects/snapfish/encoder/chromix/test.html");
    const SkBitmap &skiaBitmap = mixRender.render();

    // Encode pixel data to PNG.
    std::vector<unsigned char> pngData;
    SkAutoLockPixels bitmapLock(skiaBitmap);
    gfx::PNGCodec::Encode(reinterpret_cast<const unsigned char*>(skiaBitmap.getPixels()),
                          gfx::PNGCodec::FORMAT_BGRA, skiaBitmap.width(), skiaBitmap.height(),
                          static_cast<int>(skiaBitmap.rowBytes()), false, &pngData);

    // Write to disk.
    std::ofstream pngFile;
    pngFile.open("/tmp/render.png", std::ios::out|std::ios::trunc|std::ios::binary);
    pngFile.write(reinterpret_cast<const char *>(&pngData[0]), pngData.size());
    pngFile.close();

    [pool drain];
    return 0;
}
