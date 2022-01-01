// Copyright (c) 2021 Andrew Wason, All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FRAMESERVER_H
#define FRAMESERVER_H

#include <QObject>

namespace WebVfx
{
    class QmlContent;
}

class FrameServer : public QObject
{
    Q_OBJECT

public:
    FrameServer(QObject *parent, const QSize &size, int frameCount, const QMap<QString, QString>& propertyMap, const QUrl& qmlUrl);
    ~FrameServer();

private slots:
    void onContentLoadFinished(bool);

private:
    WebVfx::QmlContent* content;
};

#endif

