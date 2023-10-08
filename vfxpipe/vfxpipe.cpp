// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfxpipe.h"
#include <algorithm>
#include <dlfcn.h> // for dladdr, Dl_info
#include <filesystem> // for path
#include <functional> // for function
#include <limits.h> // for PATH_MAX
#include <signal.h> // for signal, SIGCHLD, SIG_IGN
#include <stdlib.h> // for exit, realpath
#include <string> // for allocator, operator+, char_traits, string
#include <tuple>
#include <unistd.h> // for close, dup2, pipe, write, execv, fork, read, STDIN_FILENO, STDOUT_FILENO

namespace VfxPipe {

std::tuple<int, uint32_t> spawnProcess(int* pipeReadStdout, int* pipeWriteStdin, int* pipeReadStderr, const std::string& url, ErrorHandler errorHandler)
{
    int fdsToChildStdin[2];
    int fdsFromChildStdout[2];
    int fdsFromChildStderr[2];

    if (pipe(fdsToChildStdin) == -1 || pipe(fdsFromChildStdout) == -1 || (pipeReadStderr && pipe(fdsFromChildStderr) == -1)) {
        errorHandler(std::string("vfxpipe pipe failed: ") + strerror(errno));
        return { -1, 0 };
    }

    // Ignore child exit so we don't have to waitpid, and to avoid zombie processes
    signal(SIGCHLD, SIG_IGN);

    int pid = fork();
    if (pid == -1) {
        errorHandler(std::string("vfxpipe fork failed: ") + strerror(errno));
        return { -1, 0 };
    }
    // In the child
    if (pid == 0) {
        if (dup2(fdsToChildStdin[0], STDIN_FILENO) == -1
            || dup2(fdsFromChildStdout[1], STDOUT_FILENO) == -1
            || (pipeReadStderr && dup2(fdsFromChildStderr[1], STDERR_FILENO) == -1)) {
            errorHandler(std::string("vfxpipe dup2 failed: ") + strerror(errno));
            exit(1);
        }

        close(fdsFromChildStdout[0]);
        close(fdsFromChildStdout[1]);
        close(fdsToChildStdin[0]);
        close(fdsToChildStdin[1]);
        if (pipeReadStderr) {
            close(fdsFromChildStderr[0]);
            close(fdsFromChildStderr[1]);
        }

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

    *pipeWriteStdin = fdsToChildStdin[1];
    close(fdsToChildStdin[0]);
    *pipeReadStdout = fdsFromChildStdout[0];
    close(fdsFromChildStdout[1]);
    if (pipeReadStderr) {
        *pipeReadStderr = fdsFromChildStderr[0];
        close(fdsFromChildStderr[1]);
    }

    uint32_t sinkCount = 0;
    if (!dataIO(*pipeReadStdout, reinterpret_cast<std::byte*>(&sinkCount), sizeof(sinkCount), read, errorHandler)) {
        return { -1, 0 };
    }

    return { pid, sinkCount };
}

FrameServer::FrameServer(const std::string& url)
    : url(url)
    , pid(0)
    , pipeWriteStdin(-1)
    , pipeReadStdout(-1)
{
}

FrameServer::~FrameServer()
{
    if (pipeReadStdout != -1)
        close(pipeReadStdout);
    if (pipeWriteStdin != -1)
        close(pipeWriteStdin);
}

bool FrameServer::initialize(ErrorHandler errorHandler, int* pipeReadStderr)
{
    if (!pid) {
        std::tie(pid, sinkCount) = spawnProcess(&pipeReadStdout, &pipeWriteStdin, pipeReadStderr, url, errorHandler);
        if (pid == -1) {
            errorHandler("vfxpipe failed to spawn process");
            return false;
        }
        return true;
    }
    return pid != -1;
}

bool FrameServer::renderFrame(double time, const std::vector<SourceVideoFrame>& sourceFrames, RenderedVideoFrame& outputFrame, ErrorHandler errorHandler)
{
    if (pid == -1)
        return false;
    if (!pid) {
        if (!initialize(errorHandler))
            return false;
    }

    auto ioErrorHandler = [this, errorHandler](std::string msg) {
        if (!msg.empty())
            errorHandler(msg);
        close(pipeReadStdout);
        pipeReadStdout = -1;
        close(pipeWriteStdin);
        pipeWriteStdin = -1;
    };

    if (!dataIO(pipeWriteStdin, reinterpret_cast<std::byte*>(&time), sizeof(time), write, ioErrorHandler)) {
        return false;
    }

    // Output format
    if (!writeVideoFrameFormat(pipeWriteStdin, outputFrame.format, ioErrorHandler)) {
        return false;
    }

    uint32_t frameCount = std::min(sinkCount, static_cast<uint32_t>(sourceFrames.size()));
    if (!dataIO(pipeWriteStdin, reinterpret_cast<const std::byte*>(&frameCount), sizeof(frameCount), write, ioErrorHandler)) {
        return false;
    }
    int index = 0;
    for (const auto& sourceFrame : sourceFrames) {
        ++index;
        if (index > frameCount)
            break;
        if (!writeVideoFrame(pipeWriteStdin, sourceFrame, ioErrorHandler)) {
            return false;
        }
    }

    if (!dataIO(pipeReadStdout, outputFrame.data, outputFrame.format.dataSize(), read, ioErrorHandler)) {
        return false;
    }
    return true;
}

}