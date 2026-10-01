#include <libavutil/avassert.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

int main(void)
{
    struct SwsContext *scaler = sws_getContext(16, 16, AV_PIX_FMT_YUV420P,
        16, 16, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
    struct SwrContext *resampler = NULL;
    const AVChannelLayout stereo = AV_CHANNEL_LAYOUT_STEREO;
    const float left[] = { 0.0f, 0.5f, -0.5f, 0.25f };
    const float right[] = { -0.25f, 0.0f, 0.5f, -0.5f };
    const uint8_t *input[] = { (const uint8_t *)left, (const uint8_t *)right };
    int16_t samples[8];
    uint8_t *output = (uint8_t *)samples;
    const int16_t expected[] = { 0, -8192, 16384, 0, -16384, 16384, 8192, -16384 };
    av_assert0(scaler);
    av_assert0(swr_alloc_set_opts2(&resampler, &stereo, AV_SAMPLE_FMT_S16, 44100,
        &stereo, AV_SAMPLE_FMT_FLTP, 44100, 0, NULL) == 0);
    av_assert0(swr_init(resampler) == 0);
    av_assert0(swr_convert(resampler, &output, 4, input, 4) == 4);
    av_assert0(!memcmp(samples, expected, sizeof(expected)));
    sws_free_context(&scaler);
    swr_free(&resampler);
    return 0;
}
