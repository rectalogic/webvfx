#include <errno.h>
#include <unistd.h>
#include <QCoreApplication>
#include "pipe_reader.h"

PipeReader::PipeReader(int fd, unsigned char* buffer, int bufferSize, QObject *parent)
    : QObject(parent)
    , notifier(new QSocketNotifier(fd, QSocketNotifier::Read, this))
    , buffer(buffer)
    , bufferSize(bufferSize)
    , currentBufferPosition(0)
{
    connect(notifier, &QSocketNotifier::activated, this, &PipeReader::dataReady);
}

void PipeReader::dataReady(QSocketDescriptor, QSocketNotifier::Type)
{
    //XXX do we need QSocketNotifier::Exception too or can we detect it when reading?

    ssize_t n = read(notifier->socket(), buffer + currentBufferPosition, bufferSize - currentBufferPosition);
    if (n == -1) {
        qCritical("read failed: %s", strerror(errno));
        QCoreApplication::exit(1);
    }
    else if (n == 0) {
        QCoreApplication::exit(0);
    }
    else {
        currentBufferPosition += n;
        if (currentBufferPosition == bufferSize - 1) {
            currentBufferPosition = 0;
            emit bufferFilled();
        }
    }
}