// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

#include <chromix/Chromix.h>
#include <chromix/MixRender.h>
#include <chromix/Delegate.h>
#include <gfx/codec/png_codec.h>


int chromix_main(int argc, const char * argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [chromium-options] <html-template-url>" << std::endl;
        return -1;
    }

    class Delegate : public Chromix::Delegate {
    public:
        virtual void logMessage(const std::string& message) {
            std::cerr << message << std::endl;
        }
        virtual v8::Handle<v8::Value> getParameterValue(const std::string& name) {
            if (name == "title")
                return wrapParameterValue(std::string("This is the title"));
            return getUndefinedParameterValue();
        }
    };

    class AutoChromix {
    public:
        AutoChromix(int argc, const char * argv[]) { Chromix::initialize(argc, argv); }
        ~AutoChromix() { Chromix::shutdown(); }
    };
    AutoChromix chromix(argc, argv);

    Chromix::MixRender mixRender(new Delegate());
    mixRender.resize(800, 600);

    if (!mixRender.loadURL(argv[argc - 1]))
        return -1;

    const int MaxFrames = 20;
    std::string key("video");
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
