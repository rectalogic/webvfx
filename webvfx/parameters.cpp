#include "webvfx/parameters.h"

WebVfx::Parameters::~Parameters()
{
}

double WebVfx::Parameters::getNumberParameter(const std::string&)
{
    return 0;
}

std::string WebVfx::Parameters::getStringParameter(const std::string&)
{
    return std::string();
}
