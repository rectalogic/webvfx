#include <iostream>
#include <fstream>

#include "MixKit.h"
#include "MixRender.h"
#include <gfx/codec/png_codec.h>

#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>

int chromix_main(int argc, const char * argv[]) {
    if (argc != 2) {
        std::cerr << "Missing html template";
        return -1;
    }
    Chromix::MixKit mixKit(argc, argv);
    Chromix::MixRender mixRender(800, 600);

    if (!mixRender.loadURL(argv[1]))
        return -1;

    unsigned char* data = mixRender.writeableDataForImageParameter(WTF::String("video"), 320, 240);
    for (unsigned int i = 0; i < 320*240*4; i += 4) {
        data[i] = 0xff; //red
        data[i+3] = 0xff; //alpha
    }

    const SkBitmap &skiaBitmap = mixRender.render(0);

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
