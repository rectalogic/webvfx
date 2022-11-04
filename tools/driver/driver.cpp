// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <getopt.h>


void usage(const char* name) {
    std::cerr << "Usage: " << name << " -s|--size <width>x<height> -f|--frames <framecount> [-h|--help] [[<input-raw-video>] ...]" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        usage(argv[0]);
        return 1;
    }

    int width = 0, height = 0;
    int frameCount;
    
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        // Render size
        {"size", required_argument, 0, 's'},
        // Number of frames to render
        {"frames", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };
    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "hs:f:",
                            long_options, &option_index)) != -1) {
        switch (c) {
        case 'h':
            usage(argv[0]);
            return 1;
        case 's': {
            char* p = strchr(optarg, 'x');
            if (!p) {
                usage(argv[0]);
                return 1;
            }
            width = strtol(optarg, NULL, 10);
            height = strtol(p + 1, NULL, 10);
            break;
        }
        case 'f':
            frameCount = strtol(optarg, NULL, 10);
            break;
        case '?':
        default:
            return 1;
        }
    }

    if (frameCount <= 0 || width <= 0 || height <= 0) {
        usage(argv[0]);
        return 1;
    }

    std::vector<std::ifstream> inputVideoStreams;
    for (int i = optind; i < argc; i++) {
        //XXX handle pipe: specifiers
        inputVideoStreams.emplace_back(argv[i], std::ifstream::binary);
    }

    int imageSize = width * height * 4;
    char image[imageSize];

    std::cout << std::unitbuf;
    for (int f = 0; f < frameCount; f++) {
        double time = (double)f / frameCount;
        std::cout.write((const char *)(&time), sizeof(double));
        for (std::istream& s : inputVideoStreams) {
            if (!s.good()) {
                std::cerr << "Input stream error" << std::endl;
                return 1;
            }
            s.read(image, imageSize);
            std::cout.write(image, imageSize);
        }
    }
 
    return 0;
}
