// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_EFFECTS_IMPL_H_
#define WEBVFX_EFFECTS_IMPL_H_

#include <QObject>
#include "webvfx/effects.h"

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
    Image getImage(const QString& name, int width, int height);
    const Image render(double time, int width, int height);
    void destroy();

private:
    ~EffectsImpl();
    Q_INVOKABLE void initializeInvokable(const QUrl& url, const QSize& size, Parameters* parameters);
    Q_INVOKABLE void renderInvokable(double time, const QSize& size);

    // Test if we are currently on the UI thread
    bool onUIThread();

    Content* content;
    Image renderImage;
    bool loadResult;
};

}

#endif
