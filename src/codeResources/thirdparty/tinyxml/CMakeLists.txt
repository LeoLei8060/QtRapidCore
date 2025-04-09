get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${DIR_NAME})

find_package(Qt5 COMPONENTS Widgets)
file(GLOB ALL_FILE "*.h" "*.cpp")
add_library(${DIR_NAME} SHARED ${ALL_FILE})

