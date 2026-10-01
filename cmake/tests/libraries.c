#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

int main(void)
{
    struct SwsContext *scaler = sws_getContext(16, 16, AV_PIX_FMT_YUV420P,
        16, 16, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
    struct SwrContext *resampler = swr_alloc();
    int failed = !scaler || !resampler;
    sws_freeContext(scaler);
    swr_free(&resampler);
    return failed;
}
