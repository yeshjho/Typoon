#include "../../Typoon/utils/config.h"
#include "../../util/config.h"


Config config;

const Config& get_config()
{
    return config;
}

void set_config(Config c)
{
    config = std::move(c);
}
