target_sources(${PROJECT_NAME} PRIVATE
    # Main
    main.cpp

    # App
    app/apppresenter.h app/apppresenter.cpp
    app/mainwindowpresenter.h app/mainwindowpresenter.cpp
    app/appconfigservice.h app/appconfigservice.cpp

    # Core
    core/crash/crashhandler.h core/crash/crashhandler.cpp
    core/database/genericdao.h

    core/event/qeventforwarder.h
    core/event/qeventforwarder.cpp

    core/log/loghelper.h
    core/log/loghelper.cpp

    core/messaging/imessages.h
    core/messaging/ithreadmessage.h
    core/messaging/threadmessagequeue.h
    core/messaging/lockfreequeue.h core/messaging/lockfreequeue.cpp

    core/network/udpmulticastmanager.h core/network/udpmulticastmanager.cpp

    core/serialization/base.h
    core/serialization/ini.h
    core/serialization/json.h
    core/serialization/xml.h
    core/serialization/o_serialize.h core/serialization/o_serialize.cpp

    core/theme/argparser.h core/theme/argparser.cpp
    core/theme/placeholderengine.h core/theme/placeholderengine.cpp
    core/theme/theme.h core/theme/theme.cpp
    core/theme/thememanager.h core/theme/thememanager.cpp
    
    core/thread/threadbase.h core/thread/threadbase.cpp
    core/thread/threadpool.h

    core/utils/singleapplication.h core/utils/singleapplication.cpp
    core/utils/utils.h
    core/utils/file_utils.cpp

    # UI
    ui/widgets/mainwindow.h ui/widgets/mainwindow.cpp ui/widgets/mainwindow.ui

    ui/components/iconbutton.h ui/components/iconbutton.cpp

    ui/services/fontmanager.h ui/services/fontmanager.cpp

    # Backend
    backend/backendmanager.h backend/backendmanager.cpp
    backend/threadmessages.h

    # Common
    common/appdefines.h
    common/iconfonts.h

    # Data
    data/messages/touiupdatemsg.h
    data/messages/topresentermsg.h
    data/config/systemconfig.h
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/core/pool
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/components
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/services
    ${CMAKE_CURRENT_SOURCE_DIR}/data
)
