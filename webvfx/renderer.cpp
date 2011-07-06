#include <QGLFormat>
#include <QGLFramebufferObject>
#include <QGLWidget>
#include <QPainter>
#include "webvfx/image.h"
#include "webvfx/content.h"
#include "webvfx/renderer.h"
#include "webvfx/webvfx.h"


namespace WebVfx
{

GLRenderer::GLRenderer(QGLWidget* glWidget)
    : glWidget(glWidget)
    , fbo(0)
{
    glWidget->makeCurrent();
    if (!QGLFramebufferObject::hasOpenGLFramebufferObjects()
        || !QGLFramebufferObject::hasOpenGLFramebufferBlit()) {
        log("Renderer: FBOs not fully supported, GL rendering will not work");
    }
    glWidget->doneCurrent();
}

GLRenderer::~GLRenderer()
{
    delete fbo;
}

void GLRenderer::createFBO(const QSize& size)
{
    if (fbo && fbo->size() == size)
        return;

    delete fbo;
    fbo = new QGLFramebufferObject(size, GL_TEXTURE_RECTANGLE_ARB);
}

bool GLRenderer::render(Content* content, Image* renderImage)
{
    if (!renderImage)
        return false;

    glWidget->makeCurrent();
    QSize size(renderImage->width(), renderImage->height());
    createFBO(size);

    // Render frame into QGLWidget.
    // Painting to anything else does not work properly.
    // https://bugs.webkit.org/show_bug.cgi?id=63946
    QPainter painter(glWidget);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    content->paintContent(&painter);
    painter.end();

    // Blit from QGLWidget to FBO, flipping image vertically
    QRect srcRect(0, 0, renderImage->width(), renderImage->height());
    QRect dstRect(0, renderImage->height(),
                  renderImage->width(), -renderImage->height());
    QGLFramebufferObject::blitFramebuffer(fbo, srcRect, 0, dstRect);

    // Read back the pixels from the FBO
    fbo->bind();
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ROW_LENGTH, renderImage->bytesPerLine() / 3);
    glReadPixels(0, 0, renderImage->width(), renderImage->height(),
                 GL_RGB, GL_UNSIGNED_BYTE, renderImage->pixels());
    glPopClientAttrib();
    fbo->release();
    glWidget->doneCurrent();

    return true;
}

GLAntialiasRenderer::GLAntialiasRenderer(QGLWidget* glWidget)
    : glWidget(glWidget)
    , multisampleFBO(0)
    , resolveFBO(0)
{
    glWidget->makeCurrent();
    if (!QGLFramebufferObject::hasOpenGLFramebufferObjects()
        || !QGLFramebufferObject::hasOpenGLFramebufferBlit()) {
        log("Renderer: FBOs not fully supported, antialiasing will not work");
    }
    glWidget->doneCurrent();
}

GLAntialiasRenderer::~GLAntialiasRenderer()
{
    delete multisampleFBO;
    delete resolveFBO;
}

void GLAntialiasRenderer::createFBOs(const QSize& size)
{
    if (multisampleFBO && resolveFBO && resolveFBO->size() == size)
        return;

    // Create a multisample FBO and an FBO to resolve into
    QGLFramebufferObjectFormat fboFormat;
    fboFormat.setSamples(4);
    fboFormat.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
    delete multisampleFBO;
    multisampleFBO = new QGLFramebufferObject(size, fboFormat);
    delete resolveFBO;
    resolveFBO = new QGLFramebufferObject(size, GL_TEXTURE_RECTANGLE_ARB);
}

bool GLAntialiasRenderer::render(Content* content, Image* renderImage)
{
    if (!renderImage)
        return false;

    glWidget->makeCurrent();
    createFBOs(QSize(renderImage->width(), renderImage->height()));

    // Render frame into multisample FBO
    QPainter painter(multisampleFBO);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    content->paintContent(&painter);
    painter.end();

    // Blit from multisample to resolve FBO.
    // Rects are setup so the image is vertically flipped when blitted
    // so when we read the pixels back they are the right way up.
    // OpenGL does everything "upside down".
    QRect srcRect(0, 0, renderImage->width(), renderImage->height());
    QRect dstRect(0, renderImage->height(),
                  renderImage->width(), -renderImage->height());
    QGLFramebufferObject::blitFramebuffer(resolveFBO, srcRect,
                                          multisampleFBO, dstRect);

    // Read back the pixels from the resolve FBO
    resolveFBO->bind();
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ROW_LENGTH, renderImage->bytesPerLine() / 3);
    glReadPixels(0, 0, renderImage->width(), renderImage->height(),
                 GL_RGB, GL_UNSIGNED_BYTE, renderImage->pixels());
    glPopClientAttrib();

    resolveFBO->release();
    glWidget->doneCurrent();

    return true;
}

bool ImageRenderer::render(Content* content, Image* renderImage)
{
    if (!renderImage)
        return false;

    // QImage referencing our Image bits
    QImage image((uchar*)renderImage->pixels(), renderImage->width(),
                 renderImage->height(), renderImage->bytesPerLine(),
                 QImage::Format_RGB888);

    // Paint into image
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    content->paintContent(&painter);
    painter.end();
    return true;
}

}
