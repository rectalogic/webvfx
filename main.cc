#include <base/basictypes.h>
#include <base/message_loop.h>
#include <gfx/codec/png_codec.h>

#include <iostream>
#include <fstream>

#include "MixKit.h"
#include "MixRender.h"

int chromix_main(int argc, const char * argv[]) {
    if (argc != 2) {
        std::cerr << "Missing html template";
        return -1;
    }
    Chromix::MixKit mixKit(argc, argv);
    Chromix::MixRender mixRender(800, 600);

    if (!mixRender.loadURL(argv[1]))
        return -1;
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

    return 0;
}
