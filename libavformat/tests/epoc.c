/*
 * EPOC Record demuxer tests
 * Copyright (c) 2026 EKA2L1 Team
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/avassert.h"
#include "libavutil/intreadwrite.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

typedef struct Input {
    uint8_t data[180];
    int size, pos;
} Input;

static int read_data(void *opaque, uint8_t *buf, int size)
{
    Input *input = opaque;
    size = FFMIN(size, input->size - input->pos);
    if (size <= 0)
        return AVERROR_EOF;
    memcpy(buf, input->data + input->pos, size);
    input->pos += size;
    return size;
}

static int64_t seek_data(void *opaque, int64_t pos, int whence)
{
    Input *input = opaque;
    if (whence == AVSEEK_SIZE)
        return input->size;
    if (whence == SEEK_CUR)
        pos += input->pos;
    else if (whence == SEEK_END)
        pos += input->size;
    else if (whence != SEEK_SET)
        return AVERROR(EINVAL);
    if (pos < 0 || pos > input->size)
        return AVERROR(EINVAL);
    return input->pos = pos;
}

static Input fixture(int cardinality_bytes, int root)
{
    Input input = { .size = 180 };
    int entry = root + cardinality_bytes;
    AV_WL32(input.data,      0x10000037);
    AV_WL32(input.data + 4,  0x1000006d);
    AV_WL32(input.data + 8,  0x1000007e);
    AV_WL32(input.data + 12, 0x5508accf);
    AV_WL32(input.data + 16, root);
    input.data[root] = cardinality_bytes == 1 ? 4 : cardinality_bytes == 2 ? 9 : 19;
    AV_WL32(input.data + entry,     0x10000052);
    AV_WL32(input.data + entry + 4, 128);
    AV_WL32(input.data + entry + 8, 0x10000089);
    AV_WL32(input.data + entry + 12, 96);
    AV_WL32(input.data + 128, 4);
    AV_WL32(input.data + 144, 4);
    memcpy(input.data + 148, "\xd5\x55\xaa\x2a", 4);
    /* A direct store can contain other streams after the audio payload. */
    memset(input.data + 152, 0xff, 28);
    return input;
}

static void check(Input input, int valid)
{
    AVFormatContext *format = avformat_alloc_context();
    AVIOContext *io = avio_alloc_context(av_malloc(4096), 4096, 0, &input,
                                        read_data, NULL, seek_data);
    AVPacket *packet = av_packet_alloc();
    int ret;
    av_assert0(format && io && packet);
    format->pb = io;
    ret = avformat_open_input(&format, NULL, valid ? NULL : av_find_input_format("epoc"), NULL);
    if (!valid) {
        av_assert0(ret < 0);
    } else {
        AVStream *stream;
        AVCodecContext *decoder;
        AVFrame *frame = av_frame_alloc();
        const int16_t expected[] = { 8, -8, 32256, -32256 };
        av_assert0(ret == 0 && format->nb_streams == 1);
        stream = format->streams[0];
        av_assert0(!strcmp(format->iformat->name, "epoc"));
        av_assert0(stream->duration == 4 && stream->time_base.num == 1 && stream->time_base.den == 8000);
        av_assert0(stream->codecpar->codec_id == AV_CODEC_ID_PCM_ALAW);
        av_assert0(stream->codecpar->ch_layout.nb_channels == 1 && stream->codecpar->sample_rate == 8000);
        av_assert0(av_read_frame(format, packet) == 0);
        av_assert0(packet->size == 4 && packet->pts == 0 && packet->duration == 4);
        decoder = avcodec_alloc_context3(avcodec_find_decoder(AV_CODEC_ID_PCM_ALAW));
        av_assert0(decoder && frame);
        av_assert0(avcodec_parameters_to_context(decoder, stream->codecpar) == 0);
        av_assert0(avcodec_open2(decoder, NULL, NULL) == 0);
        av_assert0(avcodec_send_packet(decoder, packet) == 0);
        av_assert0(avcodec_receive_frame(decoder, frame) == 0);
        av_assert0(frame->nb_samples == 4 && !memcmp(frame->data[0], expected, sizeof(expected)));
        av_packet_unref(packet);
        av_assert0(av_read_frame(format, packet) == AVERROR_EOF);
        av_assert0(av_seek_frame(format, 0, 2, 0) >= 0);
        av_assert0(av_read_frame(format, packet) == 0);
        av_assert0(packet->pts == 2 && packet->size == 2 && packet->duration == 2);
        av_assert0(packet->data[0] == 0xaa && packet->data[1] == 0x2a);
        av_frame_free(&frame);
        avcodec_free_context(&decoder);
    }
    av_packet_free(&packet);
    avformat_close_input(&format);
    av_freep(&io->buffer);
    avio_context_free(&io);
}

int main(void)
{
    Input input;
    int i;
    av_log_set_level(AV_LOG_QUIET);
    check(fixture(1, 20), 1);
    check(fixture(2, 24), 1);
    check(fixture(4, 32), 1);
    for (i = 0; i < 152; i++) {
        input = fixture(1, 20);
        input.size = i;
        check(input, 0);
    }
    input = fixture(1, 20); input.data[8] = 0; check(input, 0);
    input = fixture(1, 20); input.data[20] = 7; check(input, 0);
    input = fixture(1, 20); input.data[20] = 126; check(input, 0);
    input = fixture(1, 20); input.data[20] = 0; check(input, 0);
    input = fixture(1, 20); AV_WL32(input.data + 16, UINT32_MAX); check(input, 0);
    input = fixture(1, 20); AV_WL32(input.data + 25, UINT32_MAX); check(input, 0);
    input = fixture(1, 20); AV_WL32(input.data + 29, 0x10000052); check(input, 0);
    input = fixture(1, 20); input.data[132] = 1; check(input, 0);
    input = fixture(1, 20); input.data[128] = 5; check(input, 0);
    input = fixture(1, 20); AV_WL32(input.data + 144, UINT32_MAX); check(input, 0);
    puts("EPOC Record: all tests passed");
    return 0;
}
