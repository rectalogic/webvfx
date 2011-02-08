#include "webvfx/web_parameters.h"

WebVFX::WebParameters::~WebParameters()
{
}

double WebVFX::WebParameters::getNumberParameter(const std::string&)
{
    return 0;
}

const std::string WebVFX::WebParameters::getStringParameter(const std::string&)
{
    return std::string();
}
