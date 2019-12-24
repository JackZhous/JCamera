# 根据API版本判断使用哪个版本的OpenGLES
if (${ANDROID_PLATFORM_LEVEL} LESS 12)
    message(FATAL_ERROR "OpenGL 2 is not supported before API level 11 (currently using ${ANDROID_PLATFORM_LEVEL}).")
    return()
elseif (${ANDROID_PLATFORM_LEVEL} LESS 18)
    add_definitions("-DDYNAMIC_ES3")
    set(GLES-lib GLESv2)
else ()
    set(GLES-lib GLESv3)
endif (${ANDROID_PLATFORM_LEVEL} LESS 11)

#添加头文件
include_directories(android
                    src)

#添加源文件
add_library(player
            SHARED

        android/JNIHelp.cpp
        android/NativeHelper.cpp
        android/JMediaPlayer.cpp
        src/player/Player.cpp
        src/common/MessageQueue.cpp
        src/common/PlayerStatus.cpp
        src/common/FrameQueue.cpp
        src/common/FFmpegUtil.cpp
        src/common/PacketQueue.cpp
        src/decoder/AudioDecoder.cpp
        src/decoder/MediaDecoder.cpp
        src/decoder/VideoDecoder.cpp
        src/sync/MediaSync.cpp
        src/device/AudioDevice.cpp
        src/device/VideoDevice.cpp
        src/sync/MediaClock.cpp
        src/render/GLFilter.cpp
        src/render/OpenGLHelper.cpp
        src/render/FrameBuffer.cpp
        src/render/GLInputYUV420FIlter.cpp
        src/render/GLInputFilter.cpp
        src/render/EGLHelper.cpp
        src/render/MediaEGLContext.cpp
        src/render/RenderNode.cpp
        src/render/InputRenderNode.cpp
        src/render/CoordinateUtils.cpp
        src/player/AudioResampler.cpp
        src/render/GLInputRGBAFilter.cpp
        )




#库与库之间的共享
target_link_libraries(player
                      ffmpeg

        -landroid
        -llog
        -lOpenSLES
        -lEGL
        ${GLES-lib})