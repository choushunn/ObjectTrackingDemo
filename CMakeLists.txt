cmake_minimum_required(VERSION 3.5)

project(ObjectTrackingDemo VERSION 0.1 LANGUAGES CXX)
message(STATUS "Project Name: " ${PROJECT_NAME})
message(STATUS "current platform/architecture: ${CMAKE_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}")
set(TARGET_SOC rk3588)
# 自动
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX install)

#配置库文件路径
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(THIRD_PARTY_LIB_PATH "E:\\runtime")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(THIRD_PARTY_LIB_PATH "/root/runtime")
else()
    message(FATAL_ERROR "Unsupported current platform: ${CMAKE_SYSTEM_NAME}")
endif()

# 依赖库
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(OpenCV_DIR "${THIRD_PARTY_LIB_PATH}/opencv")
    #ToupCam_DIR
    set(ToupCam_DIR "${THIRD_PARTY_LIB_PATH}/toupcam/win")
    #ncnn
    set(ncnn_DIR "${THIRD_PARTY_LIB_PATH}/ncnn/x64/lib/cmake/ncnn")
    #onnxruntime
    #set(OnnxRuntime_DIR "${THIRD_PARTY_LIB_PATH}/onnxruntime-win-x64-gpu-1.14.1")
    include_directories("${THIRD_PARTY_LIB_PATH}/ncnn/x64/include")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
        # x86_64版本
        set(LIB_ARCH x86_64)
    elseif(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64")
        # arm64版本
        set(LIB_ARCH aarch64)
    else()
        # 不支持的处理器架构
        message(FATAL_ERROR "Unsupported processor architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
    endif()
    # 设置OpenCV路径
    set(OpenCV_DIR "${THIRD_PARTY_LIB_PATH}/opencv/lib/cmake/opencv4")
    # 设置OnnxRuntime
    set(OnnxRuntime_DIR "${THIRD_PARTY_LIB_PATH}/onnxruntime-gpu")
endif()

#导入第三方库
find_package(OpenCV REQUIRED)
find_package(ToupCam REQUIRED)
find_package(ncnn REQUIRED)
#find_package(OnnxRuntime REQUIRED)

#导入Qt库
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent)

# 设置头文件目录为include文件夹
include_directories(include)

# 设置自动生成的ui头文件的输出路径为include目录
set(CMAKE_AUTOUIC_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 设置ui文件搜索路径
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

#匹配寻找头文件
FILE(GLOB_RECURSE HEADER_FILES "${PROJECT_SOURCE_DIR}/include/*.h")
#匹配寻找源文件
FILE(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")
#匹配寻找UI文件
FILE(GLOB_RECURSE UI_FILES "${PROJECT_SOURCE_DIR}/ui/*.ui")
#翻译文件
set(TS_FILES "translations/${PROJECT_NAME}_zh_CN.ts")

set(PROJECT_SOURCES
    ${HEADER_FILES}
    ${SRC_FILES}
    ${TS_FILES}
    ${UI_FILES}
    resources/logo.rc
)


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ObjectTrackingDemo APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
    # Define properties for Android with Qt 5 after find_package() calls as:
    #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
    )
endif()
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# 拷贝models文件夹到build目录下
file(COPY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_BINARY_DIR})

#链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::WebSockets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent)
#链接第三方库
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${TOUPCAM_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE ncnn)
#target_link_libraries(${PROJECT_NAME} PRIVATE ${OnnxRuntime_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# install target and libraries
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# 复制models文件夹到安装目录下
install(DIRECTORY ${CMAKE_SOURCE_DIR}/models DESTINATION ${CMAKE_INSTALL_PREFIX})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
