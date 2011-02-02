// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMIX_WEB_SCRIPT_H_
#define CHROMIX_WEB_SCRIPT_H_

#include <QObject>

namespace Chromix
{

class Parameters;

class WebScript : public QObject
{
    Q_OBJECT
public:
    WebScript(QObject* parent = 0, Parameters* parameters = 0);
};

}

#endif
