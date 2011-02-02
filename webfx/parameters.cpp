#include "webfx/parameters.h"

WebFX::Parameters::~Parameters()
{
}

double WebFX::Parameters::getNumberParameter(const std::string&)
{
    return 0;
}

const std::string WebFX::Parameters::getStringParameter(const std::string&)
{
    return std::string();
}
