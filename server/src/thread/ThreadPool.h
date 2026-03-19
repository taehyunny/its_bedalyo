/*무한 루프를 돌며 일거리를 기다리는 Worker 스레드들, 
그리고 작업을 조율하는 std::mutex와 std::condition_variable이 포함됩니다.*/

#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

class ThreadPool {    // 스레드 풀 클래스
public:
    ThreadPool(size_t threads);  // 생성자에서 지정된 수만큼 Worker 스레드를 생성
    ~ThreadPool(); // 소멸자에서 모든 스레드를 안전하게 종료

    // 작업을 큐에 추가하는 템플릿 함수
    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args);

private:
    std::vector<std::thread> workers;   // Worker 스레드들을 보관하는 벡터
    std::queue<std::function<void()>> tasks;  // 실행할 작업들을 보관하는 큐 (std::function<void()> 형태로 모든 작업을 일반화)
    
    std::mutex queue_mutex;   // 작업 큐에 대한 동기화용 뮤텍스
    std::condition_variable condition;  // 작업이 추가될 때 Worker 스레드들을 깨우기 위한 조건 변수
    bool stop;
};

// 구현부 (간결함을 위해 헤더에 포함하거나 .cpp에 작성)
inline ThreadPool::ThreadPool(size_t threads) : stop(false) { // 생성자에서 Worker 스레드들을 시작
    for(size_t i = 0; i < threads; ++i)  // 지정된 수만큼 스레드를 생성
        workers.emplace_back([this] {  // 각 스레드는 무한 루프를 돌며 작업을 기다립니다.
            for(;;) {   // 무한 루프
                std::function<void()> task;  // 실행할 작업을 담을 변수
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex); // 큐 접근을 위한 락
                    this->condition.wait(lock, [this]{ return this->stop || !this->tasks.empty(); }); // 작업이 추가되거나 스레드 풀이 종료될 때까지 대기
                    if(this->stop && this->tasks.empty()) return; // 종료 조건: 스레드 풀이 종료되고 작업이 남아있지 않으면 루프 탈출
                    task = std::move(this->tasks.front());   // 큐에서 작업을 가져옵니다.
                    this->tasks.pop();   // 큐에서 작업 제거
                }
                task(); // 실제 핸들러 함수 실행
            }
        });
}

template<class F, class... Args>
void ThreadPool::enqueue(F&& f, Args&&... args) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    }
    condition.notify_one();
}

inline ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for(std::thread &worker: workers) worker.join();
}