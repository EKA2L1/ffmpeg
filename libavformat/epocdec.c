/*
 * EPOC Record direct-file-store demuxer
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

#include "libavutil/intreadwrite.h"
#include "libavutil/channel_layout.h"
#include "avformat.h"
#include "internal.h"
#include "demux.h"
#include "pcm.h"

typedef struct EpocContext {
    int64_t data_end;
} EpocContext;

static int epoc_probe(const AVProbeData *p)
{
    if (p->buf_size >= 20 &&
        AV_RL32(p->buf)      == 0x10000037 &&
        AV_RL32(p->buf + 4)  == 0x1000006d &&
        AV_RL32(p->buf + 8)  == 0x1000007e &&
        AV_RL32(p->buf + 12) == 0x5508accf)
        return AVPROBE_SCORE_MAX;
    return 0;
}

static int epoc_read_header(AVFormatContext *s)
{
    EpocContext *epoc = s->priv_data;
    AVIOContext *pb = s->pb;
    AVStream *st;
    uint32_t root, count, offset = 0, length, compression, stored_length;
    int64_t file_size = avio_size(pb);
    int extra, shift, i;

    if (file_size < 20 || avio_rl32(pb) != 0x10000037 ||
        avio_rl32(pb) != 0x1000006d || avio_rl32(pb) != 0x1000007e ||
        avio_rl32(pb) != 0x5508accf)
        return AVERROR_INVALIDDATA;

    root = avio_rl32(pb);
    if (root < 20 || root >= file_size || avio_seek(pb, root, SEEK_SET) < 0)
        return AVERROR_INVALIDDATA;

    /* The root dictionary uses Symbian's one-, two-, or four-byte cardinality. */
    count = avio_r8(pb);
    if (!(count & 1)) {
        extra = 0;
        shift = 1;
    } else if ((count & 3) == 1) {
        extra = 1;
        shift = 2;
    } else if ((count & 7) == 3) {
        extra = 3;
        shift = 3;
    } else {
        return AVERROR_INVALIDDATA;
    }
    for (i = 0; i < extra; i++)
        count |= (uint32_t)avio_r8(pb) << (8 * (i + 1));
    count >>= shift;
    if (avio_feof(pb) || count > (file_size - avio_tell(pb)) / 8)
        return AVERROR_INVALIDDATA;

    for (i = 0; i < count; i++) {
        uint32_t uid = avio_rl32(pb);
        uint32_t id  = avio_rl32(pb);
        if (uid == 0x10000052) {
            if (offset)
                return AVERROR_INVALIDDATA;
            offset = id;
        }
    }
    if (avio_feof(pb) || offset < 20 || offset > file_size - 20 ||
        avio_seek(pb, offset, SEEK_SET) < 0)
        return AVERROR_INVALIDDATA;

    /* WVEConv writes length, compressor UID, repeat count, silence, stored length. */
    length        = avio_rl32(pb);
    compression   = avio_rl32(pb);
    avio_skip(pb, 8);
    stored_length = avio_rl32(pb);
    if (avio_feof(pb) || stored_length > file_size - avio_tell(pb))
        return AVERROR_INVALIDDATA;
    if (compression) {
        avpriv_request_sample(s, "EPOC compressor UID 0x%08x", compression);
        return AVERROR_PATCHWELCOME;
    }
    if (length != stored_length)
        return AVERROR_INVALIDDATA;

    st = avformat_new_stream(s, NULL);
    if (!st)
        return AVERROR(ENOMEM);
    st->codecpar->codec_type            = AVMEDIA_TYPE_AUDIO;
    st->codecpar->codec_id              = AV_CODEC_ID_PCM_ALAW;
    st->codecpar->sample_rate           = 8000;
    st->codecpar->ch_layout             = (AVChannelLayout)AV_CHANNEL_LAYOUT_MONO;
    st->codecpar->bits_per_coded_sample = 8;
    st->codecpar->block_align           = 1;
    st->codecpar->bit_rate              = 64000;
    st->start_time = 0;
    st->duration = length;
    avpriv_set_pts_info(st, 64, 1, 8000);
    ffformatcontext(s)->data_offset = avio_tell(pb);
    epoc->data_end = avio_tell(pb) + stored_length;
    return 0;
}

static int epoc_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    EpocContext *epoc = s->priv_data;
    int64_t pos = avio_tell(s->pb);
    int ret;

    if (pos >= epoc->data_end)
        return AVERROR_EOF;
    ret = av_get_packet(s->pb, pkt, FFMIN(1024, epoc->data_end - pos));
    if (ret < 0)
        return ret;
    pkt->stream_index = 0;
    pkt->pts = pkt->dts = pos - ffformatcontext(s)->data_offset;
    pkt->duration = ret;
    return ret;
}

const FFInputFormat ff_epoc_demuxer = {
    .p.name         = "epoc",
    .p.long_name    = NULL_IF_CONFIG_SMALL("EPOC Record"),
    .priv_data_size = sizeof(EpocContext),
    .read_probe     = epoc_probe,
    .read_header    = epoc_read_header,
    .read_packet    = epoc_read_packet,
    .read_seek      = ff_pcm_read_seek,
};
