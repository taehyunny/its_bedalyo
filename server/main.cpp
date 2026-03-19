#include <iostream>
#include <exception>
#include "src/network/ItsServer.h"
#include "session/ClientSession.h"
#include "database/UserDAO.h"
#include "database/AuthDAO.h"
#include "handler/UserHandler.h"
#include "../../common/dto/AllDTOs.h"
#include "thread/ThreadPool.h"

// 1. 전역 스레드풀 선언 (Dispatcher가 이 녀석을 찾아서 씁니다)
// 워커 스레드 4개를 생성합니다. (CPU 코어 수에 맞추는 것이 좋습니다)
ThreadPool g_threadPool(4);

int main(int argc, char *argv[])
{
    // 기본 포트를 8080으로 설정, 실행 시 인자로 변경 가능하게 처리
    int port = 8080;
    if (argc > 1)
    {
        try
        {
            port = std::stoi(argv[1]);
        }
        catch (const std::exception &e)
        {
            std::cerr << "[WARNING] 잘못된 포트 번호. 기본값(8080)을 사용합니다." << std::endl;
        }
    }

    std::cout << "===========================================" << std::endl;
    std::cout << "       [ Its Bedalyo Server Booting ]      " << std::endl;
    std::cout << "===========================================" << std::endl;

    try
    {
        // 2. 서버 엔진 생성 (포트 바인딩 및 epoll 초기화)
        ItsServer server(port);

        // 3. 메인 이벤트 루프 시작 (여기서 블로킹되어 계속 실행됨)
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "[FATAL] 서버 실행 중 예외 발생: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}