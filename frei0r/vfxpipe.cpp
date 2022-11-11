// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfxpipe.h"
#include <cstring>
#include <dlfcn.h>
#include <errno.h>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <wordexp.h>

void replaceAll(std::string& inout, std::string_view what, std::string_view with)
{
    for (std::string::size_type pos {};
         inout.npos != (pos = inout.find(what.data(), pos, what.length()));
         pos += with.length()) {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
}

class VfxPipe {
public:
    VfxPipe(unsigned int width, unsigned int height)
        : width(width)
        , height(height)
        , frameSize(width * height * 4)
        , pid(0)
        , pipeWrite(-1)
        , pipeRead(-1)
    {
    }

    ~VfxPipe()
    {
        if (pipeRead != -1)
            close(pipeRead);
        if (pipeWrite != -1)
            close(pipeWrite);
    }

    void setCommandLine(const char* commandLine)
    {
        if (this->commandLine.empty()) {
            this->commandLine = commandLine;
        }
    }

    std::string& getCommandLine()
    {
        return this->commandLine;
    }

    void updateFrame(
        double time,
        const uint32_t* inframe1,
        const uint32_t* inframe2,
        const uint32_t* inframe3,
        uint32_t* outframe)
    {
        spawnProcess();

        if (!dataIO(pipeWrite, reinterpret_cast<std::byte*>(&time), sizeof(time), write)) {
            return;
        }

        if (inframe1 != nullptr) {
            if (!dataIO(pipeWrite, reinterpret_cast<const std::byte*>(inframe1), frameSize, write)) {
                return;
            }
        }
        if (inframe2 != nullptr) {
            if (!dataIO(pipeWrite, reinterpret_cast<const std::byte*>(inframe2), frameSize, write)) {
                return;
            }
        }
        if (inframe3 != nullptr) {
            if (!dataIO(pipeWrite, reinterpret_cast<const std::byte*>(inframe3), frameSize, write)) {
                return;
            }
        }

        if (!dataIO(pipeRead, reinterpret_cast<std::byte*>(outframe), frameSize, read)) {
            return;
        }
    }

private:
    int pid;
    int pipeWrite;
    int pipeRead;
    std::string commandLine;
    unsigned int width;
    unsigned int height;
    unsigned int frameSize;

    void spawnProcess()
    {
        if (pid)
            return;

        int fdsToChild[2];
        int fdsFromChild[2];

        if (pipe(fdsToChild) == -1 || pipe(fdsFromChild) == -1) {
            std::cerr << __FUNCTION__ << ": vfxpipe pipe failed: " << strerror(errno) << std::endl;
            pid = -1;
            return;
        }

        // Ignore child exit so we don't have to waitpid, and to avoid zombie processes
        signal(SIGCHLD, SIG_IGN);

        pid = fork();
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

            std::string commandLineTemplate(commandLine);
            replaceAll(commandLineTemplate, "{{width}}", std::to_string(width));
            replaceAll(commandLineTemplate, "{{height}}", std::to_string(height));
            wordexp_t we;
            int werr = wordexp(commandLineTemplate.c_str(), &we, WRDE_NOCMD);
            if (werr != 0) {
                std::cerr << __FUNCTION__ << ": vfxpipe wordexp failed: " << werr << std::endl;
                exit(1);
            }

            // Allowlist
            Dl_info info;
            auto fptr = &f0r_init;
            if (dladdr(reinterpret_cast<void*&>(fptr), &info)) {
                std::cerr << "allowlist " << info.dli_fname << std::endl;
                // XXX check that we.we_wordv[0] is in our allowlist
            }

            if (execvp(we.we_wordv[0], we.we_wordv) < 0) {
                std::cerr << __FUNCTION__ << ": vfxpipe exec '" << commandLineTemplate << "' failed: " << strerror(errno) << std::endl;
                exit(1);
            }
            // No need to wordfree()
        }

        // In the parent

        pipeWrite = fdsToChild[1];
        pipeRead = fdsFromChild[0];

        close(fdsFromChild[1]);
        close(fdsToChild[0]);
    }

    template <typename D, typename T>
    bool dataIO(int fd, D data, size_t size, T ioFunc)
    {
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

void get_common_plugin_info(f0r_plugin_info_t* info)
{
    info->author = "Andrew Wason";
    info->color_model = F0R_COLOR_MODEL_RGBA8888;
    info->frei0r_version = FREI0R_MAJOR_VERSION;
    info->major_version = 0;
    info->minor_version = 9;
    info->num_params = 1;
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
    return static_cast<f0r_instance_t>(new VfxPipe(width, height));
}

void f0r_destruct(f0r_instance_t instance)
{
    VfxPipe* vfxpipe = static_cast<VfxPipe*>(instance);
    delete vfxpipe;
}

void f0r_set_param_value(f0r_instance_t instance, f0r_param_t param, int param_index)
{
    if (param_index != 0)
        return;
    VfxPipe* vfxpipe = static_cast<VfxPipe*>(instance);
    vfxpipe->setCommandLine(*(static_cast<f0r_param_string*>(param)));
}

void f0r_get_param_value(f0r_instance_t instance, f0r_param_t param, int param_index)
{
    if (param_index != 0)
        return;
    VfxPipe* vfxpipe = static_cast<VfxPipe*>(instance);
    *static_cast<f0r_param_string*>(param) = const_cast<f0r_param_string>(vfxpipe->getCommandLine().c_str());
}

void f0r_update2(
    f0r_instance_t instance,
    double time,
    const uint32_t* inframe1,
    const uint32_t* inframe2,
    const uint32_t* inframe3,
    uint32_t* outframe)
{
    VfxPipe* vfxpipe = static_cast<VfxPipe*>(instance);
    vfxpipe->updateFrame(time, inframe1, inframe2, inframe3, outframe);
}

void f0r_update(
    f0r_instance_t instance,
    double time,
    const uint32_t* inframe,
    uint32_t* outframe)
{
    f0r_update2(instance, time, inframe, 0, 0, outframe);
}