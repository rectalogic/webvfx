// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_CONTENT_H_
#define WEBVFX_CONTENT_H_

#include "webvfx/effects.h"

class QSize;
class QString;
class QUrl;

namespace WebVfx
{

class Image;

class Content
{
public:
    Content() {};
    virtual ~Content() = 0;

    // This should synchronously load content
    virtual bool loadContent(const QUrl& url) = 0;
    virtual void setContentSize(const QSize& size) = 0;
    virtual const Effects::ImageTypeMap& getImageTypeMap() = 0;
    virtual Image getImage(const QString& name, const QSize& size) = 0;
    virtual bool renderContent(double time, Image* renderImage) = 0;
};

}

#endif
