cmake_minimum_required(VERSION 3.16)
project(EChatServer)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)
find_package(Boost 1.71 COMPONENTS system thread filesystem REQUIRED)
find_package(Threads REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} .)

add_executable(echatserver
    main.cpp
    ServerApp.cpp
    HttpRegistration.cpp
    Auth.cpp
    UserDB.cpp
    SyncService.cpp
    AdminConsole.cpp
    ConfigUtil.cpp
)

target_link_libraries(echatserver
    Boost::system Boost::thread Boost::filesystem
    OpenSSL::SSL OpenSSL::Crypto
    pthread
)