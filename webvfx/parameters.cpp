#include "parameters.h"
#include <QString>

WebVfx::Parameters::~Parameters()
{
}

double WebVfx::Parameters::getNumberParameter(const QString&)
{
    return 0;
}

QString WebVfx::Parameters::getStringParameter(const QString&)
{
    return QString();
}
