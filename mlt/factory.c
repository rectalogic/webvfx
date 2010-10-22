#include <mlt/framework/mlt.h>
#include "chromix_helper.h"

MLT_REPOSITORY
{
    chromix_register_services(repository, producer_type);
    chromix_register_services(repository, filter_type);
    chromix_register_services(repository, transition_type);
}
