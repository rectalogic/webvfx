// Copyright (c) 2011 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

class QString;

namespace WebVfx {

/*!
 * @brief Callback interface to expose logging.
 *
 * An instance of this class should be passed to WebVfx::setLogger().
 * WebVfx will then log all messages using that instance.
 */
class Logger {
public:
    virtual ~Logger() = 0;

    /*!
     * @brief Called whenever WebVfx needs to log a message.
     *
     * @param msg Message to be logged.
     */
    // XXX should add log level enum
    virtual void log(const QString& msg) = 0;
};

}
