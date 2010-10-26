#ifndef MOTIONBOX_MLTCHROMIX_CHROMIX_HELPER_H_
#define MOTIONBOX_MLTCHROMIX_CHROMIX_HELPER_H_

#ifdef __cplusplus
extern "C" {
#endif

void chromix_register_services(mlt_repository repository, mlt_service_type service_type);
int chromix_set_image(mlt_properties properties, const char* track, uint8_t* image, int width, int height);
int chromix_render(mlt_properties properties, double time, uint8_t* image, int width, int height);

mlt_frame chromix_filter_process(mlt_filter self, mlt_frame frame);
int chromix_producer_get_frame(mlt_producer producer, mlt_frame_ptr frame, int index);
mlt_frame chromix_transition_process(mlt_transition transition, mlt_frame a_frame, mlt_frame b_frame);

#ifdef __cplusplus
}
#endif

#endif
