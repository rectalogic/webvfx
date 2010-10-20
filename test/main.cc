#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <chromix/MixKit.h>
#include <chromix/MixRender.h>
#include <gfx/codec/png_codec.h>

#include <base/string16.h>
#include <third_party/WebKit/JavaScriptCore/wtf/text/WTFString.h>

void chromix_log(string16 const& msg, const void*) {
    std::cerr << msg << std::endl;
}

int chromix_main(int argc, const char * argv[]) {
    if (argc != 2) {
        std::cerr << "Missing html template";
        return -1;
    }
    Chromix::MixKit mixKit(argc, argv);
    Chromix::MixRender mixRender;
    mixRender.setLogger(chromix_log);
    mixRender.resize(800, 600);

    if (!mixRender.loadURL(WTF::String::fromUTF8(argv[1])))
        return -1;

    const int MaxFrames = 20;
    WTF::String key = WTF::String::fromUTF8("video");
    for (int f = 0; f < MaxFrames; f++) {
        unsigned char* data = mixRender.writeableDataForImageParameter(key, 320, 240);
        if (!data)
            return -1;
        for (unsigned int i = 0; i < 320*240*4; i += 4) {
            data[i] = 0xff * ((double)f / MaxFrames); //shade of red
            data[i+3] = 0xff; //alpha
        }

        const SkBitmap* skiaBitmap = mixRender.render((double)f / MaxFrames);
        if (!skiaBitmap)
            return -1;

        // Encode pixel data to PNG.
        std::vector<unsigned char> pngData;
        SkAutoLockPixels bitmapLock(*skiaBitmap);
        gfx::PNGCodec::Encode(reinterpret_cast<const unsigned char*>(skiaBitmap->getPixels()),
                              gfx::PNGCodec::FORMAT_BGRA, skiaBitmap->width(), skiaBitmap->height(),
                              static_cast<int>(skiaBitmap->rowBytes()), false, &pngData);

        // Write to disk.
        std::ostringstream fileName;
        fileName << "/tmp/render/render" << std::right << std::setfill('0') << std::setw(2) << f << ".png";
        std::ofstream pngFile;
        pngFile.open(fileName.str().c_str(), std::ios::out|std::ios::trunc|std::ios::binary);
        if (pngFile.fail())
            return -1;
        pngFile.write(reinterpret_cast<const char *>(&pngData[0]), pngData.size());
        pngFile.close();
    }

    return 0;
}
