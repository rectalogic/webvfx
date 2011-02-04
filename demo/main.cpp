// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

#include <webfx/webfx.h>


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <html-template-url>" << std::endl;
        return -1;
    }

    class Logger : public WebFX::WebLogger {
    public:
        virtual void log(const std::string& message) {
            std::cerr << message << std::endl;
        }
    };
    WebFX::setLogger(new Logger());

    class Parameters : public WebFX::WebParameters {
    public:
        virtual const std::string getStringParameter(const std::string&) {
            return "WebFX Cool Title";
        }
    };

    class AutoWebFX {
    public:
        //XXX check return code
        AutoWebFX() { WebFX::initialize(); }
        ~AutoWebFX() { WebFX::shutdown(); }
    };
    AutoWebFX fx;

    WebFX::WebEffects* effects = WebFX::createWebEffects();
    effects->initialize(argv[1], 320, 240, new Parameters());
    WebFX::WebImage video = effects->getImage("video", 320, 240);
    // Fill with red XXX need to take into account stride
    unsigned char* pixels = video.pixels();
    for (int i = 0; i < video.byteCount(); i+= WebFX::WebImage::BytesPerPixel) {
        pixels[i] = 0xFF;
        pixels[i+1] = 0x00;
        pixels[i+2] = 0x00;
    }
    const WebFX::WebImage image = effects->render(0.32, 320, 240);

    // Write to disk.
    std::ofstream rawFile;
    rawFile.open("/tmp/webfx.raw", std::ios::out|std::ios::trunc|std::ios::binary);
    if (rawFile.fail())
        return -1;
    rawFile.write(reinterpret_cast<const char*>(image.pixels()), image.byteCount());
    rawFile.close();

    /*XXX
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
    */

    return 0;
}
