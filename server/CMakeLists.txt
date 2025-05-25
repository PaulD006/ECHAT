cmake_minimum_required(VERSION 3.16)
project(EChatServer)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread filesystem)
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
)


target_link_libraries(echatserver
    OpenSSL::SSL OpenSSL::Crypto
    ${Boost_LIBRARIES}
    Threads::Threads
)