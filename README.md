# FFmpeg README

FFmpeg is a collection of libraries and tools to process multimedia content
such as audio, video, subtitles and related metadata.

## Building for EKA2L1

EKA2L1 adds this directory through CMake. FFmpeg is compiled on demand from
source; no platform libraries or installed headers are tracked in this fork.
Bash and GNU Make are required. On Windows, install MSYS2 with its `make`
package alongside Visual Studio's C++ tools (`C:/msys64` is detected by default).
For another installation location, set `EKA2L1_FFMPEG_BASH` and
`EKA2L1_FFMPEG_MAKE` to its `usr/bin/bash.exe` and `usr/bin/make.exe`.

The cache defaults to the main project's `build/ffmpeg-cache`. Override it with
`EKA2L1_FFMPEG_CACHE_DIR`; `EKA2L1_FFMPEG_JOBS` controls compilation parallelism.
Source contents, revision, compiler, architecture, SDK path and configure flags
select separate cache entries. Builds sharing an entry are serialized by a lock.
Deleting the cache causes a rebuild. Do not configure FFmpeg in its source tree;
run `make distclean` there if an earlier manual build left configuration files.

A standalone native build also runs the EPOC Record decoder tests:

```sh
cmake -S . -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
```

## Libraries

* `libavcodec` provides implementation of a wider range of codecs.
* `libavformat` implements streaming protocols, container formats and basic I/O access.
* `libavutil` includes hashers, decompressors and miscellaneous utility functions.
* `libavfilter` provides means to alter decoded audio and video through a directed graph of connected filters.
* `libavdevice` provides an abstraction to access capture and playback devices.
* `libswresample` implements audio mixing and resampling routines.
* `libswscale` implements color conversion and scaling routines.

## Tools

* [ffmpeg](https://ffmpeg.org/ffmpeg.html) is a command line toolbox to
  manipulate, convert and stream multimedia content.
* [ffplay](https://ffmpeg.org/ffplay.html) is a minimalistic multimedia player.
* [ffprobe](https://ffmpeg.org/ffprobe.html) is a simple analysis tool to inspect
  multimedia content.
* Additional small tools such as `aviocat`, `ismindex` and `qt-faststart`.

## Documentation

The offline documentation is available in the **doc/** directory.

The online documentation is available in the main [website](https://ffmpeg.org)
and in the [wiki](https://trac.ffmpeg.org).

### Examples

Coding examples are available in the **doc/examples** directory.

## License

FFmpeg codebase is mainly LGPL-licensed with optional components licensed under
GPL. Please refer to the LICENSE file for detailed information.

## Contributing

Patches should be submitted to the ffmpeg-devel mailing list using
`git format-patch` or `git send-email`. Github pull requests should be
avoided because they are not part of our review process and will be ignored.
