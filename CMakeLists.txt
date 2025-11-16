cmake_minimum_required(VERSION 3.16)

project(CryptoMW LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets WebSockets Network)

add_executable(CryptoMW
    main.cpp
    mainwindow.cpp
    mainwindow.h
    globals.cpp
    globals.h
    version.h
    login.h login.cpp login.ui
    marketwatchdockwindow.h marketwatchdockwindow.cpp
    marketwatchmodel.h marketwatchmodel.cpp
    websocketconnection.h websocketconnection.cpp
    protocol.h
    configmanager.h configmanager.cpp
    orderbookwindow.h orderbookwindow.cpp orderbookwindow.ui
    README.md



)

target_link_libraries(CryptoMW
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebSockets
    Qt6::Network

)
