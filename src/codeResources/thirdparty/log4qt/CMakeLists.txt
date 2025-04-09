get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${DIR_NAME})

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Network)

file(GLOB_RECURSE HEADER_FILES "*.h")
add_library(${DIR_NAME} SHARED IMPORTED ${HEADER_FILES})
add_custom_target(${DIR_NAME}_headers SOURCES ${HEADER_FILES})
