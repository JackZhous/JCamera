set(common_dir ../common)

# 添加头文件路径
include_directories(${common_dir} )

#添加头文件
include_directories(android
        src)

#添加源文件
add_library(recorder
            SHARED

            # 通用类
            ${common_dir}/AVFrameFilter.cpp
            ${common_dir}/AVMediaWriter.cpp
            ${common_dir}/AVFormatter.cpp
            ${common_dir}/AVMediaData.cpp
            ${common_dir}/YUVConvertor.cpp
            ${common_dir}/YUVData.cpp

            android/RecorderNative.cpp
            src/Recorder.cpp
            src/RecordParams.cpp
        )




#库与库之间的共享
target_link_libraries(recorder
        yuv
        ffmpeg

        -landroid
        -llog
        -lOpenSLES
        -lEGL
        ${GLES-lib})