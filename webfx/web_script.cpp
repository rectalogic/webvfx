#include "webfx/web_parameters.h"
#include "webfx/web_page.h"
#include "webfx/web_script.h"

WebFX::WebScript::WebScript(WebFX::WebPage* parent, WebFX::WebParameters* parameters)
    : QObject(parent)
    , parameters(parameters)
{
}

WebFX::WebScript::~WebScript()
{
    delete parameters;
}

void WebFX::WebScript::render(double time)
{
    emit renderRequested(time);
}

double WebFX::WebScript::getNumberParameter(const QString& name)
{
    if (parameters)
        return parameters->getNumberParameter(name.toStdString());
    else
        return 0;
}

const QString WebFX::WebScript::getStringParameter(const QString& name)
{
    if (parameters)
        return QString::fromStdString(parameters->getStringParameter(name.toStdString()));
    else
        return QString();
}
