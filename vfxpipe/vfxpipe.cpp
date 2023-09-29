// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

#ifndef WEBVFX_EXECUTABLE
#define WEBVFX_EXECUTABLE "webvfx"
#endif

namespace VfxPipe {

int spawnProcess(int* pipeRead, int* pipeWrite, const std::string& url, std::function<void(std::string)> errorHandler)
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

        char* const argv[] = { const_cast<char*>(WEBVFX_EXECUTABLE), const_cast<char*>(url.c_str()), nullptr };
        if (execvp(WEBVFX_EXECUTABLE, argv) < 0) {
            errorHandler(std::string("vfxpipe exec '") + WEBVFX_EXECUTABLE + " " + url + "' failed: " + strerror(errno));
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

}