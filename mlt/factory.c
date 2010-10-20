#include <mlt/framework/mlt.h>

extern mlt_filter filter_chromix_init(mlt_profile profile, mlt_service_type type, const char *id, char *arg);

MLT_REPOSITORY
{
    MLT_REGISTER(filter_type, "Chromix", filter_chromix_init);
    //XXX also producer and transition
}
