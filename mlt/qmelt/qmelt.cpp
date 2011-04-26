// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#undef main

#include <QApplication>
#include <QThread>

extern "C" int melt_main(int argc, char** argv);

class MeltThread : public QThread
{
public:
    MeltThread(int argc, char** argv)
        : QThread(), argc(argc), argv(argv) {
    }

protected:
    void run() {
        qApp->exit(melt_main(argc, argv));
    }

private:
    int argc;
    char** argv;
};


int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MeltThread meltThread(argc, argv);
    meltThread.start();
    return app.exec();
}
