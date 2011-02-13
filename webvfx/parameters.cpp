#include "webvfx/parameters.h"

WebVFX::Parameters::~Parameters()
{
}

double WebVFX::Parameters::getNumberParameter(const std::string&)
{
    return 0;
}

std::string WebVFX::Parameters::getStringParameter(const std::string&)
{
    return std::string();
}
