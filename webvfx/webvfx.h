// Copyright (c) 2010 Hewlett-Packard Development Company, L.P. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBVFX_WEBVFX_H_
#define WEBVFX_WEBVFX_H_

#include <webvfx/effects.h>
#include <webvfx/logger.h>
#include <webvfx/parameters.h>

/*!
 * @brief Public entry points into WebVfx
 *
 * These are the methods and classes that will be needed to host
 * WebVfx effect rendering.
 */
namespace WebVfx {

/*!
 * @brief Set a Logger implementation
 *
 * WebVfx will take ownership of the logger and delete it at shutdown.
 * setLogger() must be called only once and must be called before initialize().
 */
void setLogger(Logger* logger);

/*!
 * @brief Log a message
 *
 * Logs a message using the currently set Logger implementation.
 */
void log(const QString& msg);

/*!
 * @brief Initialize the WebVfx framework
 *
 * initialize() is threadsafe and may be called multiple times.
 * It must be called before calling createEffects() for the first time.
 *
 * On MacOS, in a non Qt host application, initialize() and processEvents()
 * must be called from the main thread.
 * In a Qt based MacOS application, or a non-MacOS application,
 * initialize() can be called from any thread and processEvents()
 * need not be called.
 * See <a href="http://bugreports.qt.nokia.com/browse/QTBUG-7393">QTBUG-7393</a>.
 *
 * @return Indicates whether initialization was successful
 */
bool initialize();

/*!
 * @brief Create an Effects instance
 *
 * @param fileName Path to a QML or HTML effects implementation.
 *   @c fileName must end in @c .html, @c .htm, or @c .qml
 * @param width Initial width of effect in pixels
 * @param height Initial height of effect in pixels
 * @param parameters Parameters implementation to provide named parameter
 *   values for this effect
 */
Effects* createEffects(const QString& fileName, int width, int height, Parameters* parameters = 0, bool isTransparent = false);

/*!
 * @brief Workaround for MacOS
 *
 * Must be called on the main thread after initialize() in
 * non-Qt based MacOS applications if Effects is going to be used
 * from any other threads.
 * It is a noop on other platforms or on Qt-based MacOS applications.
 * @note This method will block until shutdown() is called.
 */
int processEvents();

/*!
 * @brief Shut down WebVfx.
 *
 * All Effects should be destroyed before calling shutdown().
 * shutdown() can be called from any thread.
 * It is safe to call shutdown() more than once,
 * or without having called initialize().
 */
void shutdown();

}
#endif
