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

Renderer::~Renderer()
{
    deleteFBOs();
}

// Renderer does not take ownership of glWidget
void Renderer::init(QGLWidget* glWidget, const QSize& size)
{
    this->glWidget = glWidget;
    resize(size);
}

void Renderer::setRenderType(RenderType type)
{
    if (renderType == RenderGLAntialias && type != RenderGLAntialias)
        deleteFBOs();
    renderType = type;
}

void Renderer::resize(const QSize& size) {
    if (size == this->size)
        return;
    this->size = size;
    deleteFBOs();
}

void Renderer::deleteFBOs()
{
    delete multisampleFBO; multisampleFBO = 0;
    delete resolveFBO; resolveFBO = 0;
}

bool Renderer::createFBOs()
{
    if (!glWidget)
        return false;

    if (multisampleFBO && resolveFBO)
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
    multisampleFBO = new QGLFramebufferObject(size, fboFormat);
    resolveFBO = new QGLFramebufferObject(size, GL_TEXTURE_RECTANGLE_ARB);

    glWidget->doneCurrent();
    return true;
}

bool Renderer::render(Content* content, Image* renderImage)
{
    if (!renderImage)
        return false;

    switch (renderType) {
    case RenderGL:
    default:
        return renderGL(content, renderImage);
    case RenderGLAntialias:
        return renderGLAntialias(content, renderImage);
    case RenderNoGL:
        return renderNoGL(content, renderImage);
    }
}

bool Renderer::renderGL(Content* content, Image* renderImage)
{
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

bool Renderer::renderGLAntialias(Content* content, Image* renderImage)
{
    createFBOs();

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

bool Renderer::renderNoGL(Content* content, Image* renderImage)
{
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
