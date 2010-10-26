#include <mlt/framework/mlt.h>
#include "chromix_helper.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXPORT __attribute__((visibility ("default")))
#else
#define EXPORT
#endif

EXPORT MLT_REPOSITORY
{
    chromix_register_services(repository, producer_type);
    chromix_register_services(repository, filter_type);
    chromix_register_services(repository, transition_type);
}
