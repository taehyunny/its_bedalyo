#pragma once
#include <unordered_map>
#include "ClientSession.h"

class SessionManager {
public: // 이 키워드가 반드시 있어야 합니다!
    void createSession(int fd) {
        sessionMap[fd] = new ClientSession(fd);
    }

    ClientSession* getSession(int fd) {
        if (sessionMap.find(fd) == sessionMap.end()) return nullptr;
        return sessionMap[fd];
    }

    void removeSession(int fd) {
        auto it = sessionMap.find(fd);
        if (it != sessionMap.end()) {
            delete it->second;
            sessionMap.erase(it);
        }
    }

private:
    std::unordered_map<int, ClientSession*> sessionMap;
};