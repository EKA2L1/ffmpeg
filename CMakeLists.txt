if (NOT DEFINED FFMPEG_CORE_NAME)
	set(FFMPEG_CORE_NAME ffmpeg)
endif()

add_library(${FFMPEG_CORE_NAME} INTERFACE)

target_include_directories(${FFMPEG_CORE_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
if (WIN32)
	target_link_libraries(${FFMPEG_CORE_NAME} INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}/windows/x86_64/lib/avformat.lib"
		"${CMAKE_CURRENT_SOURCE_DIR}/windows/x86_64/lib/avcodec.lib"
		"${CMAKE_CURRENT_SOURCE_DIR}/windows/x86_64/lib/swscale.lib"
		"${CMAKE_CURRENT_SOURCE_DIR}/windows/x86_64/lib/avutil.lib"
		"${CMAKE_CURRENT_SOURCE_DIR}/windows/x86_64/lib/swresample.lib"
		"Bcrypt.lib")
elseif (APPLE)
	target_link_libraries(${FFMPEG_CORE_NAME} INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}/macos/x86_64/lib/libavformat.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/macos/x86_64/lib/libavcodec.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/macos/x86_64/lib/libswscale.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/macos/x86_64/lib/libavutil.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/macos/x86_64/lib/libswresample.a"
		"z")
elseif (ANDROID)
	target_link_libraries(${FFMPEG_CORE_NAME} INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}/android/${CMAKE_ANDROID_ARCH_ABI}/lib/libavformat.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/android/${CMAKE_ANDROID_ARCH_ABI}/lib/libavcodec.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/android/${CMAKE_ANDROID_ARCH_ABI}/lib/libswscale.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/android/${CMAKE_ANDROID_ARCH_ABI}/lib/libavutil.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/android/${CMAKE_ANDROID_ARCH_ABI}/lib/libswresample.a"
		"z")
elseif (UNIX)
	target_link_libraries(${FFMPEG_CORE_NAME} INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}/linux/x86_64/lib/libavformat.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/linux/x86_64/lib/libavcodec.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/linux/x86_64/lib/libswscale.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/linux/x86_64/lib/libavutil.a"
		"${CMAKE_CURRENT_SOURCE_DIR}/linux/x86_64/lib/libswresample.a"
		"z")
endif()