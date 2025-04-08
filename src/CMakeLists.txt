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
