// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_EFFECTS_IMPL_H_
#define WEBVFX_EFFECTS_IMPL_H_

#include <QObject>
#include "webvfx/effects.h"

class QMutex;
class QWaitCondition;
class QSize;
class QString;
class QUrl;

namespace WebVfx
{

class Content;
class Image;
class Parameters;

class EffectsImpl : public QObject, public Effects
{
    Q_OBJECT
public:
    EffectsImpl();

    // EffectsImpl will take ownership of Parameters
    bool initialize(const QString& fileName, int width, int height, Parameters* parameters = 0);
    const ImageTypeMap& getImageTypeMap();
    void setImage(const QString& name, Image* image);
    bool render(double time, Image* renderImage);
    void destroy();

private slots:
    void initializeComplete(bool result);

private:
    ~EffectsImpl();
    Q_INVOKABLE void initializeInvokable(const QUrl& url, const QSize& size, Parameters* parameters, bool isPlain);
    Q_INVOKABLE void renderInvokable(double time, Image* renderImage);

    // Test if we are currently on the UI thread
    bool onUIThread();

    Content* content;
    QMutex* mutex;
    QWaitCondition* waitCondition;
    bool initializeResult;
    bool renderResult;
};

}

#endif
