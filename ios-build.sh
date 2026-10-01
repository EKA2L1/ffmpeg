#!/bin/sh
#
# Build the trimmed ffmpeg the emulator needs for iOS, as static libraries.
#
# Unlike the desktop scripts this one builds out of tree, because the two iOS
# slices (device and simulator) cannot share one configured source directory.
# Each slice installs into ios/<slice>, next to macos/ and android/.
#
# Usage: ./ios-build.sh [device|simulator|all]   (default: all)

set -e

FFMPEG_SRC="$(cd "$(dirname "$0")" && pwd)"
DEPLOYMENT_TARGET="${EKA2L1_IOS_DEPLOYMENT_TARGET:-16.0}"
JOBS="$(sysctl -n hw.logicalcpu 2>/dev/null || echo 4)"

build_one() {
    slice="$1"
    sdk="$2"
    min_flag="$3"

    prefix="${FFMPEG_SRC}/ios/${slice}"
    build_dir="${FFMPEG_SRC}/build-ios-${slice}"
    sdk_path="$(xcrun --sdk "${sdk}" --show-sdk-path)"

    echo "==> Configuring ffmpeg for iOS ${slice} (${sdk})"

    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"
    cd "${build_dir}"

    "${FFMPEG_SRC}/configure" \
        --prefix="${prefix}" \
        --target-os=darwin \
        --arch=arm64 \
        --cpu=armv8-a \
        --cc="$(xcrun --sdk "${sdk}" -f clang)" \
        --cxx="$(xcrun --sdk "${sdk}" -f clang++)" \
        --sysroot="${sdk_path}" \
        --enable-cross-compile \
        --extra-cflags="-arch arm64 -isysroot ${sdk_path} ${min_flag}=${DEPLOYMENT_TARGET} -Os -D__STDC_CONSTANT_MACROS" \
        --extra-ldflags="-arch arm64 -isysroot ${sdk_path} ${min_flag}=${DEPLOYMENT_TARGET}" \
        --disable-everything \
        --disable-shared \
        --enable-static \
        --enable-pic \
        --disable-asm \
        --disable-avdevice \
        --disable-filters \
        --disable-programs \
        --disable-network \
        --disable-avfilter \
        --disable-postproc \
        --disable-encoders \
        --disable-doc \
        --disable-ffplay \
        --disable-ffprobe \
        --disable-ffmpeg \
        --enable-zlib \
        --enable-decoder=h264 \
        --enable-decoder=mpeg4 \
        --enable-decoder=h263 \
        --enable-decoder=h263p \
        --enable-decoder=mpeg2video \
        --enable-decoder=mjpeg \
        --enable-decoder=mjpegb \
        --enable-decoder=aac \
        --enable-decoder=aac_latm \
        --enable-decoder=wavpack \
        --enable-decoder=amrnb \
        --enable-decoder=amrwb \
        --enable-decoder=amr \
        --enable-decoder=mp3 \
        --enable-decoder=pcm_alaw \
        --enable-decoder=pcm_s16le \
        --enable-decoder=pcm_s8 \
        --enable-encoder=pcm_s16le \
        --enable-demuxer=h264 \
        --enable-demuxer=m4v \
        --enable-demuxer=mp3 \
        --enable-demuxer=mpegvideo \
        --enable-demuxer=mpegps \
        --enable-demuxer=mjpeg \
        --enable-demuxer=mov \
        --enable-demuxer=avi \
        --enable-demuxer=aac \
        --enable-demuxer=amr \
        --enable-demuxer=amrnb \
        --enable-demuxer=amrwb \
        --enable-demuxer=epoc \
        --enable-demuxer=pcm_s16le \
        --enable-demuxer=pcm_s8 \
        --enable-demuxer=wav \
        --enable-muxer=amr \
        --enable-muxer=avi \
        --enable-muxer=mp3 \
        --enable-muxer=wav \
        --enable-muxer=pcm_s16le \
        --enable-muxer=pcm_s8 \
        --enable-muxer=ogg \
        --enable-parser=h264 \
        --enable-parser=mpeg4video \
        --enable-parser=mpegvideo \
        --enable-parser=aac \
        --enable-parser=aac_latm \
        --enable-parser=mpegaudio \
        --enable-protocol=file

    echo "==> Building ffmpeg for iOS ${slice}"
    make -j"${JOBS}"
    make install

    cd "${FFMPEG_SRC}"
    rm -rf "${build_dir}"
}

case "${1:-all}" in
    simulator)
        build_one simulator iphonesimulator -mios-simulator-version-min
        ;;
    device)
        build_one device iphoneos -miphoneos-version-min
        ;;
    all)
        build_one device iphoneos -miphoneos-version-min
        build_one simulator iphonesimulator -mios-simulator-version-min
        ;;
    *)
        echo "Unknown target: $1" >&2
        echo "Usage: $0 [device|simulator|all]" >&2
        exit 2
        ;;
esac
