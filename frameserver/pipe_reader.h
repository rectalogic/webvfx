// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QObject>
#include <QSocketNotifier>

class PipeReader : public QObject
{
    Q_OBJECT
public:
    explicit PipeReader(int fd, unsigned char* buffer, int bufferSize, QObject *parent = 0);
signals:
    void bufferFilled();
public slots:
    void dataReady(QSocketDescriptor sockfd, QSocketNotifier::Type sntype);
private:
    QSocketNotifier* notifier;
    unsigned char* buffer;
    int bufferSize;
    int currentBufferPosition;
};