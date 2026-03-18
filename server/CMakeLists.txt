cmake_minimum_required(VERSION 3.10)
project(ItsServer_App)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/network)
include_directories(src/handler)
include_directories(src/session)
include_directories(src/database)
include_directories(src/thread)
# 현재 위치에 main.cpp가 있으므로 그냥 main.cpp라고 적습니다.
# 나머지 파일들은 src/ 폴더 안에 있으므로 경로를 명시합니다.
set(SOURCES
    main.cpp
    src/network/ItsServer.cpp
    src/network/PacketFramer.cpp
    src/handler/Dispatcher.cpp
    src/handler/OrderHandler.cpp
    src/handler/SystemHandler.cpp
    src/handler/UserHandler.cpp
    src/database/ConnectionPool.cpp
    src/database/OrderDAO.cpp
    src/database/UserDAO.cpp
)

add_executable(ItsServer_App ${SOURCES})

target_link_libraries(ItsServer_App mysqlcppconn pthread)
