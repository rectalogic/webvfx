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

bool GLRenderer::render(Content* content, Image* renderImage)
{
    if (!renderImage)
        return false;

    glWidget->makeCurrent();

    // Render frame into QGLWidget
    QPainter painter(glWidget);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    content->paintContent(&painter);
    painter.end();

    // Read back the pixels
    glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ROW_LENGTH, renderImage->bytesPerLine() / 3);
    glReadPixels(0, 0, renderImage->width(), renderImage->height(),
                 GL_RGB, GL_UNSIGNED_BYTE, renderImage->pixels());
    glPopClientAttrib();

    glWidget->doneCurrent();

    return true;
}

GLAntialiasRenderer::~GLAntialiasRenderer()
{
    delete multisampleFBO;
    delete resolveFBO;
}

bool GLAntialiasRenderer::createFBOs(const QSize& size)
{
    if (!glWidget)
        return false;

    if (multisampleFBO && resolveFBO && resolveFBO->size() == size)
        return true;

    if (!QGLFramebufferObject::hasOpenGLFramebufferObjects()
        || !QGLFramebufferObject::hasOpenGLFramebufferBlit()) {
        log("Renderer: FBOs not fully supported, antialiasing will not work");
        return false;
    }

    // Create a multisample FBO and an FBO to resolve into
    glWidget->makeCurrent();

    QGLFramebufferObjectFormat fboFormat;
    fboFormat.setSamples(4);
    fboFormat.setTextureTarget(GL_TEXTURE_RECTANGLE_ARB);
    fboFormat.setAttachment(QGLFramebufferObject::CombinedDepthStencil);
    delete multisampleFBO;
    multisampleFBO = new QGLFramebufferObject(size, fboFormat);
    delete resolveFBO;
    resolveFBO = new QGLFramebufferObject(size, GL_TEXTURE_RECTANGLE_ARB);

    glWidget->doneCurrent();
    return true;
}

bool GLAntialiasRenderer::render(Content* content, Image* renderImage)
{
    if (!renderImage)
        return false;

    createFBOs(QSize(renderImage->width(), renderImage->height()));

    glWidget->makeCurrent();

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
