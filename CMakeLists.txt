cmake_minimum_required(VERSION 3.5)

project(ObjectTrackingDemo VERSION 0.1 LANGUAGES CXX)
message(STATUS "Project Name: " ${PROJECT_NAME})
message(STATUS "current platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "current architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")

#配置库文件路径
if(WIN32)
    set(THIRD_PARTY_LIB_PATH "E:\\runtime")
else()
    set(THIRD_PARTY_LIB_PATH "/home/mc/Projects/runtime")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX install)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(OpenCV_DIR "${THIRD_PARTY_LIB_PATH}/opencv")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    #配置OpenCV路径
    set(OpenCV_DIR "${THIRD_PARTY_LIB_PATH}/opencv/lib/cmake/opencv4")
    #设置OnnxRuntime
    set(OnnxRuntime_DIR "${THIRD_PARTY_LIB_PATH}/onnxruntime-gpu")
endif()

#设置ToupCam_DIR和ToupCamConfig
set(ToupCam_DIR "${THIRD_PARTY_LIB_PATH}/toupcam")
#ncnn
set(ncnn_DIR "${THIRD_PARTY_LIB_PATH}/ncnn/x64/lib/cmake/ncnn")

#导入第三方库
find_package(OpenCV REQUIRED)
find_package(ToupCam REQUIRED)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    find_package(ncnn REQUIRED)
    include_directories("${THIRD_PARTY_LIB_PATH}/ncnn/x64/include")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
#    find_package(OnnxRuntime REQUIRED)
endif()

#导入Qt库
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent)
#翻译文件
set(TS_FILES ${PROJECT_NAME}_zh_CN.ts)

#自动寻找源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST)
#匹配寻找头文件
FILE(GLOB_RECURSE HEADER_LIST "${PROJECT_SOURCE_DIR}/src/*.h")
#匹配寻找头文件
FILE(GLOB_RECURSE UI_LIST "${PROJECT_SOURCE_DIR}/src/*.ui")

set(PROJECT_SOURCES
    ${HEADER_LIST}
    ${SRC_LIST}
    ${UI_LIST}
    ${TS_FILES}
    logo.rc
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

#链接Qt库
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::SerialPort)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::WebSockets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent)
#链接第三方库
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ToupCam_LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OnnxRuntime_LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE ncnn)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

message(${CMAKE_INSTALL_PREFIX})
install(DIRECTORY  ${PROJECT_SOURCE_DIR}/models/ DESTINATION models)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()
