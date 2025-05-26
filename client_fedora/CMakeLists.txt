cmake_minimum_required(VERSION 3.16)
project(EChatClient)
set(CMAKE_CXX_STANDARD 17)

# Enable Qt’s AUTOMOC so Q_OBJECT gets processed
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(OpenSSL REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets Network REQUIRED)

add_executable(echatclient
    main.cpp
    LoginDialog.cpp
    MainWindow.cpp
    NetworkClient.cpp
    Crypto.cpp
    Storage.cpp
)

target_link_libraries(echatclient
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    OpenSSL::Crypto
    OpenSSL::SSL
)