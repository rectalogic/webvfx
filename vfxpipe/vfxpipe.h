// Copyright (c) 2022 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <functional>
#include <string>
#include <unistd.h>

void replaceAll(std::string& inout, std::string_view what, std::string_view with);

int spawnProcess(int* pipeRead, int* pipeWrite, std::string& commandLine, std::function<void(std::string)> errorHandler);

template <typename D, typename IO, typename ERR>
bool dataIO(int fd, D data, size_t size, IO ioFunc, ERR errFunc)
{
    size_t bytesIO = 0;
    while (bytesIO < size) {
        ssize_t n = ioFunc(fd, data + bytesIO, size - bytesIO);
        // EOF
        if (n == 0) {
            errFunc(n);
            return false;
        }
        if (n == -1) {
            errFunc(n, std::string("vfxpipe data IO failed: ") + strerror(errno));
            return false;
        }
        bytesIO = bytesIO + n;
    }
    return true;
}