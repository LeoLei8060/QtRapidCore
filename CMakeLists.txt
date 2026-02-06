cmake_minimum_required(VERSION 3.16)

project(QtRapidCore VERSION 1.0.0 LANGUAGES CXX)

# 自动生成版本信息
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h"
)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 检测操作系统平台
if(WIN32)
    set(PLATFORM "Windows")
elseif(APPLE)
    set(PLATFORM "MacOS")
elseif(UNIX)
    set(PLATFORM "Linux")
else()
    set(PLATFORM "Unknown")
    message(WARNING "无法识别的操作系统平台")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(COMPILE_MODE "debug")
else()
    set(COMPILE_MODE "release")
endif()

# 检测架构（32/64位）
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "x64")
else()
    set(ARCH "x86")
endif()

# log4qt
add_definitions(-DUSE_LOG4QT -DO_SERIALIZE_USE_QT)

# 设置参数
set(THIRDPARTY_LIB_PATH     ${CMAKE_SOURCE_DIR}/3rdparty/lib/${ARCH}/${COMPILE_MODE}/)
set(COPY_DLL_DIR            ${CMAKE_SOURCE_DIR}/3rdparty/lib/${ARCH}/${COMPILE_MODE}/)
set(COPY_THEMES_DIR         ${CMAKE_SOURCE_DIR}/themes/)
set(COPY_CONFIG_FILE        ${CMAKE_CURRENT_SOURCE_DIR}/log.conf)
set(COPY_RES_DIR            ${CMAKE_SOURCE_DIR}/res/)

# 修改生成路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

# 查找Qt包
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Network)

set(3RDPARTY
    3rdparty/nameof/nameof.hpp
)

# 定义可执行文件 (源文件在 src/CMakeLists.txt 中添加)
if (WIN32)
    find_package(Qt5 REQUIRED COMPONENTS Core)
    add_executable(${PROJECT_NAME} WIN32 ${3RDPARTY})
    target_link_libraries(QtRapidCore PRIVATE Qt5::Core)
else()
    add_executable(${PROJECT_NAME} ${3RDPARTY})
endif()

if (WIN32)
    set(LOG4_LIB_PATH ${THIRDPARTY_LIB_PATH}/liblog4qt.a)
elseif (UNIX)
    set(LOG4_LIB_PATH ${THIRDPARTY_LIB_PATH}/liblog4qt.so)
endif()

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    pthread
    ${LOG4_LIB_PATH}
)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE dbghelp ws2_32)
endif()

# 引用库目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/log4qt
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/serialization
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 添加源码子目录
add_subdirectory(src)

############################### 拷贝文件
# 拷贝DLL
if(EXISTS "${COPY_DLL_DIR}")
  file(COPY ${COPY_DLL_DIR} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  message(FATAL_ERROR "not found lib dir, path:${COPY_DLL_DIR}")
endif()

# 拷贝主题文件夹
if(EXISTS "${COPY_THEMES_DIR}")
  file(COPY ${COPY_THEMES_DIR} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/themes/)
else()
  message(FATAL_ERROR "not found styles dir, path:${COPY_THEMES_DIR}")
endif()

# 拷贝资源
if(EXISTS "${COPY_RES_DIR}")
  file(COPY ${COPY_RES_DIR} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/)
else()
  message(FATAL_ERROR "not found res dir, path:${COPY_RES_DIR}")
endif()

# 拷贝配置文件
file(COPY ${COPY_CONFIG_FILE} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# 项目构建配置
if(BUILD_TESTS)
    add_subdirectory(tests)
    message(STATUS "Building test projects")
else()
    message(STATUS "Skipping test projects")
endif()
