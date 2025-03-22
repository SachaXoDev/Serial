cmake_minimum_required(VERSION 3.19)
project(untitled LANGUAGES CXX)

# Найти необходимые модули Qt (добавлен Sql)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

qt_standard_project_setup()

# Создание исполняемого файла
qt_add_executable(untitled
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    registrationwindow.h
    registrationwindow.cpp
    registrationwindow.ui
    home.h home.cpp home.ui


)

# Подключение модулей Qt (добавлен Qt::Sql)
target_link_libraries(untitled
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
)

include(GNUInstallDirs)

# Установка приложения
install(TARGETS untitled
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Генерация скрипта для развертывания
qt_generate_deploy_app_script(
    TARGET untitled
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
