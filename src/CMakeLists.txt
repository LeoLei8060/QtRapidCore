set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    projectpreviewdialog.cpp
    projectpreviewdialog.h
    projectpreviewdialog.ui
    projectcreator.cpp
    projectcreator.h
)

add_executable(QtRapidCore
    ${PROJECT_SOURCES}
)

# 链接Qt库
target_link_libraries(QtRapidCore PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

# 设置安装目录
install(TARGETS QtRapidCore
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_command(TARGET QtRapidCore POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/codeResources"
    "$<TARGET_FILE_DIR:QtRapidCore>/codeResources"
    COMMENT "Copying resource folder to output directory"
)

add_custom_command(TARGET QtRapidCore POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/../templates"
    "$<TARGET_FILE_DIR:QtRapidCore>/templates"
    COMMENT "Copying templates folder to output directory"
)
