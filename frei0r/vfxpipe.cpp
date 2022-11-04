// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
extern "C" {
    #include <frei0r.h>
}

extern char **environ;

struct VfxPipe {
    int pipeWrite;
    int pipeRead;
    bool initialized;
    std::string commandLine;
    unsigned int width;
    unsigned int height;
    unsigned int frameSize;

    template <typename D, typename T>
    bool dataIO(int fd, D data, size_t size, T ioFunc) {
        size_t bytesIO = 0;
        while (bytesIO < size) {
            ssize_t n = ioFunc(fd, data + bytesIO, size - bytesIO);
            // EOF
            if (n == 0) {
                close(pipeRead);
                pipeRead = -1;
                close(pipeWrite);
                pipeWrite = -1;
                return false;
            }
            if (n == -1) {
                std::cerr << __FUNCTION__ << ": vfxpipe Image IO failed: " << strerror(errno) << std::endl;
                close(pipeRead);
                pipeRead = -1;
                close(pipeWrite);
                pipeWrite = -1;
                return false;
            }
            bytesIO = bytesIO + n;
        }
        return true;
    }
};


int f0r_init()
{
    return 1;
}

void f0r_deinit()
{
}

void f0r_get_param_info(f0r_param_info_t* info, int param_index)
{
    if (param_index == 0) {
        info->name = "Command Line";
        info->type = F0R_PARAM_STRING;
        info->explanation = "Command line for the process to spawn and pipe frames to/from";
    }
}

f0r_instance_t f0r_construct(unsigned int width, unsigned int height)
{
    VfxPipe *vfxpipe = new VfxPipe();
    vfxpipe->width = width;
    vfxpipe->height = height;
    vfxpipe->frameSize = width * height * 4;
    vfxpipe->pipeWrite = -1;
    vfxpipe->pipeRead = -1;
    return static_cast<f0r_instance_t>(vfxpipe);
}

void f0r_destruct(f0r_instance_t instance)
{
    VfxPipe *vfxpipe = static_cast<VfxPipe*>(instance);
    if (vfxpipe->pipeRead != -1)
        close(vfxpipe->pipeRead);
    if (vfxpipe->pipeWrite != -1)
        close(vfxpipe->pipeWrite);
    delete vfxpipe;
}

void f0r_set_param_value(f0r_instance_t instance, f0r_param_t param, int param_index)
{
    if (param_index != 0)
        return;
    VfxPipe *vfxpipe = static_cast<VfxPipe*>(instance);
    vfxpipe->commandLine = *(static_cast<f0r_param_string *>(param));
}

void f0r_get_param_value(f0r_instance_t instance, f0r_param_t param, int param_index)
{
    if (param_index != 0)
        return;
    VfxPipe *vfxpipe = static_cast<VfxPipe*>(instance);
    *static_cast<f0r_param_string *>(param) = const_cast<f0r_param_string>(vfxpipe->commandLine.c_str());
 }

void spawnProcess(VfxPipe *vfxpipe)
{
    vfxpipe->initialized = true;

    int fdsToChild[2];
    int fdsFromChild[2];

    if (pipe(fdsToChild) == -1
        || pipe(fdsFromChild) == -1) {
        std::cerr << __FUNCTION__ << ": vfxpipe pipe failed: " << strerror(errno) << std::endl;
        return;
    }

    // Ignore child exit so we don't have to waitpid, and to avoid zombie processes
    signal(SIGCHLD, SIG_IGN);

    int pid = fork();
    if (pid == -1) {
        std::cerr << __FUNCTION__ << ": vfxpipe fork failed: " << strerror(errno) << std::endl;
        return;
    }
    // In the child
    if (pid == 0) {
        if (dup2(fdsToChild[0], STDIN_FILENO) == -1
            || dup2(fdsFromChild[1], STDOUT_FILENO) == -1) {
            std::cerr << __FUNCTION__ << ": vfxpipe dup2 failed: " << strerror(errno) << std::endl;
            exit(1);
        }

		close(fdsFromChild[0]);
		close(fdsFromChild[1]);
		close(fdsToChild[0]);
		close(fdsToChild[1]);

        auto envWidth = std::string("VFXPIPE_WIDTH=") + std::to_string(vfxpipe->width);
        auto envHeight = std::string("VFXPIPE_HEIGHT=") + std::to_string(vfxpipe->height);
        const char * const envExtra[] = {
            envWidth.c_str(),
            envHeight.c_str(),
            NULL,
        };
        char **p;
        int environSize;
        for (p = environ, environSize = 0; *p != NULL; p++, environSize++);
        char const * envp[environSize + std::size(envExtra)];
        for (auto i = 0; i < environSize; i++) {
            envp[i] = environ[i];
        }
        for (size_t i = environSize, j = 0; j < std::size(envExtra); i++, j++) {
            envp[i] = envExtra[j];
        }
        auto execCommand = std::string("exec ") + vfxpipe->commandLine;
        const char * const argv[] = {
            "/bin/sh",
            "-c",
            execCommand.c_str(),
            NULL,
        };
        if (execve(argv[0], const_cast<char * const *>(argv), const_cast<char * const *>(envp)) < 0) {
            std::cerr << __FUNCTION__ << ": vfxpipe exec failed: " << strerror(errno) << std::endl;
            exit(1);
        }
    }

    // In the parent

    vfxpipe->pipeWrite = fdsToChild[1];
    vfxpipe->pipeRead = fdsFromChild[0];

    close(fdsFromChild[1]);
	close(fdsToChild[0]);
}

void f0r_update2(
    f0r_instance_t instance,
    double time,
    const uint32_t* inframe1,
    const uint32_t* inframe2,
    const uint32_t* inframe3,
    uint32_t* outframe)
{
    VfxPipe *vfxpipe = static_cast<VfxPipe*>(instance);
    if (!vfxpipe->initialized) {
        spawnProcess(vfxpipe);
    }

    if (!vfxpipe->dataIO(vfxpipe->pipeWrite, reinterpret_cast<std::byte *>(&time), sizeof(time), write)) {
        return;
    }

    if (inframe1 != nullptr) {
        if (!vfxpipe->dataIO(vfxpipe->pipeWrite, reinterpret_cast<const std::byte *>(inframe1), vfxpipe->frameSize, write)) {
            return;
        }
    }
    if (inframe2 != nullptr) {
        if (!vfxpipe->dataIO(vfxpipe->pipeWrite, reinterpret_cast<const std::byte *>(inframe2), vfxpipe->frameSize, write)) {
            return;
        }
    }
    if (inframe3 != nullptr) {
        if (!vfxpipe->dataIO(vfxpipe->pipeWrite, reinterpret_cast<const std::byte *>(inframe3), vfxpipe->frameSize, write)) {
            return;
        }
    }

    if (!vfxpipe->dataIO(vfxpipe->pipeRead, reinterpret_cast<std::byte *>(outframe), vfxpipe->frameSize, read)) {
        return;
    }
}

void f0r_update(
    f0r_instance_t instance,
    double time,
    const uint32_t* inframe,
    uint32_t* outframe)
{
    f0r_update2(instance, time, inframe, 0, 0, outframe);
}