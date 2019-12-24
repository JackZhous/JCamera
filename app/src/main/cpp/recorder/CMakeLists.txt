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
add_library(recorder
            SHARED

            android/RecorderNative.cpp
            src/Recorder.cpp
        )




#库与库之间的共享
target_link_libraries(recorder
        player
        ffmpeg

        -landroid
        -llog
        -lOpenSLES
        -lEGL
        ${GLES-lib})