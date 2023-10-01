// Copyright (c) 2023 Andrew Wason. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QObject>
#include <QUrl>

class ContentPipe : public QObject {
public:
    ContentPipe(const QUrl& url, QObject* parent = nullptr);
    ~ContentPipe();
    bool renderContent(
        double time,
        const QList<QImage> sourceImages,
        QImage& outputImage);

private:
    QUrl url;
    int pid;
    int pipeWrite;
    int pipeRead;
};
