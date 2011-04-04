// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <getopt.h>
#include <webvfx/webvfx.h>

void usage(const char* name) {
    std::cerr << "Usage: " << name << " [-h|--help] [-p|--parameter <name>=<value>]... [-n|--source-name <source-image-name>] -o|--output <output-directory> <html-or-qml-filename>" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        usage(argv[0]);
        return 1;
    }

    std::map<const QString, const QString> parameterMap;
    QString sourceName;
    std::string outputDirectory;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        // Parameter name=value to expose to effect
        {"parameter", required_argument, 0, 'p'},
        // Name of source video image
        {"source-name", required_argument, 0, 'n'},
        // Output directory to write images to
        {"output", required_argument, 0, 'o'},
        {0, 0, 0, 0}
    };
    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "hp:n:o:",
                            long_options, &option_index)) != -1) {
        switch (c) {
        case 'h':
            usage(argv[0]);
            return 1;
        case 'p': {
            char* p = strchr(optarg, '=');
            if (!p) {
                usage(argv[0]);
                return 1;
            }
            QString name(QString::fromUtf8(optarg, p - optarg));
            QString value(QString::fromUtf8(p + 1));
            parameterMap.insert(std::pair<const QString, const QString>(name, value));
            break;
        }
        case 'n':
            sourceName = optarg;
            break;
        case 'o':
            outputDirectory = optarg;
            outputDirectory.append("/");
            break;
        case '?':
        default:
            return 1;
        }
    }

    if (outputDirectory.empty() || optind >= argc) {
        usage(argv[0]);
        return 1;
    }

    const char* effectFile = argv[optind];

    class Logger : public WebVfx::Logger {
    public:
        void log(const QString& message) {
            std::cerr << message.toStdString() << std::endl;
        }
    };
    WebVfx::setLogger(new Logger());

    class Parameters : public WebVfx::Parameters {
    public:
        Parameters(std::map<const QString, const QString> &map) : map(map) {}
        QString getStringParameter(const QString& name) {
            return map[name];
        }
    private:
        std::map<const QString, const QString>& map;
    };

    class AutoWebVfx {
    public:
        //XXX check return code
        AutoWebVfx() { WebVfx::initialize(); }
        ~AutoWebVfx() { WebVfx::shutdown(); }
    };
    AutoWebVfx vfx;

    const int width = 320;
    const int height = 240;

    WebVfx::Effects* effects = WebVfx::createEffects(effectFile, width, height, new Parameters(parameterMap));
    if (!effects) {
        std::cerr << "Failed to create Effects" << std::endl;
        return 1;
    }

    const int MaxFrames = 20;

    for (int f = 0; f < MaxFrames; f++) {

        WebVfx::Image video = effects->getImage(sourceName, width, height);
        unsigned char* pixels = video.pixels();
        if (!pixels) {
            std::cerr << "Failed to get video pixels" << std::endl;
            return 1;
        }
        double time = (double)f / MaxFrames;
        // Fill with shade of red XXX need to take into account stride
        for (int i = 0; i < video.byteCount(); i+= WebVfx::Image::BytesPerPixel) {
            pixels[i] = 0xFF * time;
            pixels[i+1] = 0x00;
            pixels[i+2] = 0x00;
        }
        const WebVfx::Image image = effects->render(time, width, height);

        // Write to disk.
        std::stringstream oss;
        oss << outputDirectory << "webvfx" << f << ".raw";
        std::string rawFileName = oss.str();
        std::ofstream rawFile;
        rawFile.open(rawFileName.c_str(), std::ios::out|std::ios::trunc|std::ios::binary);
        if (rawFile.fail()) {
            std::cerr << "Failed to write file " << rawFileName << std::endl;
            return 1;
        }
        rawFile.write(reinterpret_cast<const char*>(image.pixels()),
                      image.byteCount());
        rawFile.close();
    }

    effects->destroy(); effects = 0;

    return 0;
}
