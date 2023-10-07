// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfxpipe.h"
#include <dlfcn.h> // for dladdr, Dl_info
#include <filesystem> // for path
#include <functional> // for function
#include <limits.h> // for PATH_MAX
#include <signal.h> // for signal, SIGCHLD, SIG_IGN
#include <stdlib.h> // for exit, realpath
#include <string> // for allocator, operator+, char_traits, string
#include <unistd.h> // for close, dup2, pipe, write, execv, fork, read, STDIN_FILENO, STDOUT_FILENO

namespace VfxPipe {

int spawnProcess(int* pipeRead, int* pipeWrite, const std::string& url, ErrorHandler errorHandler)
{
    int fdsToChild[2];
    int fdsFromChild[2];

    if (pipe(fdsToChild) == -1 || pipe(fdsFromChild) == -1) {
        errorHandler(std::string("vfxpipe pipe failed: ") + strerror(errno));
        return -1;
    }

    // Ignore child exit so we don't have to waitpid, and to avoid zombie processes
    signal(SIGCHLD, SIG_IGN);

    int pid = fork();
    if (pid == -1) {
        errorHandler(std::string("vfxpipe fork failed: ") + strerror(errno));
        return -1;
    }
    // In the child
    if (pid == 0) {
        if (dup2(fdsToChild[0], STDIN_FILENO) == -1
            || dup2(fdsFromChild[1], STDOUT_FILENO) == -1) {
            errorHandler(std::string("vfxpipe dup2 failed: ") + strerror(errno));
            exit(1);
        }

        close(fdsFromChild[0]);
        close(fdsFromChild[1]);
        close(fdsToChild[0]);
        close(fdsToChild[1]);

        Dl_info info;
        auto fptr = &spawnProcess;
        if (dladdr(reinterpret_cast<void*&>(fptr), &info)) {
            std::filesystem::path path(info.dli_fname);
            path.remove_filename();
            path /= "webvfx";
            char resolvedPath[PATH_MAX];
            if (!realpath(path.c_str(), resolvedPath)) {
                errorHandler(std::string("vfxpipe realpath '") + path.generic_string() + "' failed: " + strerror(errno));
                exit(1);
            }
            char* const argv[] = { const_cast<char*>(resolvedPath), const_cast<char*>(url.c_str()), nullptr };
            if (execv(argv[0], argv) < 0) {
                errorHandler(std::string("vfxpipe exec '") + argv[0] + " " + url + "' failed: " + strerror(errno));
                exit(1);
            }
        } else {
            errorHandler(std::string("vfxpipe unable to determine library path"));
            exit(1);
        }
    }

    // In the parent

    *pipeWrite = fdsToChild[1];
    *pipeRead = fdsFromChild[0];

    close(fdsFromChild[1]);
    close(fdsToChild[0]);
    return pid;
}

FrameServer::FrameServer(const std::string& url)
    : url(url)
    , pid(0)
    , pipeWrite(-1)
    , pipeRead(-1)
{
}

FrameServer::~FrameServer()
{
    if (pipeRead != -1)
        close(pipeRead);
    if (pipeWrite != -1)
        close(pipeWrite);
}

bool FrameServer::renderFrame(double time, const std::vector<SourceVideoFrame>& sourceFrames, RenderedVideoFrame& outputFrame, ErrorHandler errorHandler)
{
    if (!pid) {
        pid = spawnProcess(&pipeRead, &pipeWrite, url, errorHandler);
    }
    if (pid == -1) {
        errorHandler("vfxpipe failed to spawn process");
        return false;
    }

    auto ioErrorHandler = [this, errorHandler](std::string msg) {
        if (!msg.empty())
            errorHandler(msg);
        close(pipeRead);
        pipeRead = -1;
        close(pipeWrite);
        pipeWrite = -1;
    };

    if (!dataIO(pipeWrite, reinterpret_cast<std::byte*>(&time), sizeof(time), write, ioErrorHandler)) {
        return false;
    }

    // Output format
    if (!writeVideoFrameFormat(pipeWrite, outputFrame.format, ioErrorHandler)) {
        return false;
    }

    uint32_t frameCount = sourceFrames.size();
    if (!dataIO(pipeWrite, reinterpret_cast<const std::byte*>(&frameCount), sizeof(frameCount), write, ioErrorHandler)) {
        return false;
    }
    for (const auto& sourceFrame : sourceFrames) {
        if (!writeVideoFrame(pipeWrite, sourceFrame, ioErrorHandler)) {
            return false;
        }
    }

    if (!dataIO(pipeRead, outputFrame.data, outputFrame.format.dataSize(), read, ioErrorHandler)) {
        return false;
    }
    return true;
}

}