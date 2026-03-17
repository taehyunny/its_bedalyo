cmake_minimum_required(VERSION 3.10)
project(Its_Bedalyo_Server)

# C++ 17 표준 사용 (nlohmann/json 및 최신 문법 대응)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================================
# 핵심 설정: 헤더 파일 찾기 경로 추가
# ==========================================================
# 1. ${CMAKE_CURRENT_SOURCE_DIR}/src : ItsServer.h 등을 찾기 위함
# 2. ${CMAKE_CURRENT_SOURCE_DIR}/../common : 프로젝트 루트의 공용 폴더를 찾기 위함
include_directories(
    src
    src/network
    src/handler
    src/session
    src/database
    ../common
)

# 소스 파일들 등록 (모든 .cpp 파일을 나열하거나 패턴으로 추가)
file(GLOB RECURSIVE SOURCES "src/*.cpp")

# 실행 파일 생성
add_executable(ItsServer_App ${SOURCES})

# nlohmann_json 같은 라이브러리 연결 (이미 설치되어 있거나 include에 있다면 자동 인식)
# target_link_libraries(ItsServer_App pthread) # 리눅스 통신 시 필수