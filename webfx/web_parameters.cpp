#include "webfx/web_parameters.h"

WebFX::WebParameters::~WebParameters()
{
}

double WebFX::WebParameters::getNumberParameter(const std::string&)
{
    return 0;
}

const std::string WebFX::WebParameters::getStringParameter(const std::string&)
{
    return std::string();
}
