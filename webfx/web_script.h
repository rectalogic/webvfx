// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBFX_WEB_SCRIPT_H_
#define WEBFX_WEB_SCRIPT_H_

#include <QObject>

namespace WebFX
{

class WebParameters;

class WebScript : public QObject
{
    Q_OBJECT
public:
    WebScript(QObject* parent = 0, WebParameters* parameters = 0);
};

}

#endif
