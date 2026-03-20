#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include "Global_protocol.h"
#include "json.hpp"

#pragma comment(lib, "ws2_32.lib")

using json = nlohmann::json;

#define WM_PACKET_RECEIVED  (WM_USER + 100)

struct ReceivedPacket
{
    CmdID       cmdId = CmdID::REQ_HEARTBEAT;
    std::string body;
};

class CNetworkHelper
{
public:
    CNetworkHelper()
    {
        WSADATA wsa;
        m_wsaOk = (WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
    }

    ~CNetworkHelper()
    {
        Disconnect();
        WSACleanup();
    }
    bool IsValid() const { return m_wsaOk; }

    bool Connect(const std::string& ip, int port, HWND hNotify)
    {
        if (!m_wsaOk) return false;
        if (m_connected.load()) return false;

        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_socket == INVALID_SOCKET) return false;

        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<u_short>(port));
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        if (connect(m_socket,
            reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return false;
        }

        m_hNotify = hNotify;
        m_connected = true;

        m_recvThread = std::thread(&CNetworkHelper::RecvLoop, this);

        return true;
    }

    void Disconnect()
    {
        if (!m_connected.load()) return;
        m_connected = false;

        if (m_socket != INVALID_SOCKET)
        {
            shutdown(m_socket, SD_BOTH);
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
        }

        if (m_recvThread.joinable())
            m_recvThread.join();
    }

    bool IsConnected() const { return m_connected.load(); }

    bool Send(CmdID cmdId, const json& bodyJson)
    {
        if (!m_connected.load()) return false;

        std::string bodyStr = bodyJson.dump();

        PacketHeader header;
        memset(&header, 0, sizeof(header));
        header.signature = 0x4543;
        header.cmdId = cmdId;
        header.bodySize = static_cast<uint32_t>(bodyStr.size());

        if (send(m_socket,
            reinterpret_cast<const char*>(&header),
            sizeof(header), 0) == SOCKET_ERROR) return false;

        if (!bodyStr.empty())
        {
            if (send(m_socket, bodyStr.c_str(),
                static_cast<int>(bodyStr.size()), 0) == SOCKET_ERROR)
                return false;
        }

        return true;
    }

private:
    void RecvLoop()
    {
        while (m_connected.load())
        {
            PacketHeader header;
            if (!RecvExact(reinterpret_cast<char*>(&header), sizeof(header)))
            {
                m_connected = false;
                break;
            }

            if (header.signature != 0x4543) continue;

            // Use vector<char> instead of string to avoid const data() issue
            std::vector<char> bodyBuf;
            if (header.bodySize > 0)
            {
                bodyBuf.resize(header.bodySize);
                if (!RecvExact(bodyBuf.data(), header.bodySize))
                {
                    m_connected = false;
                    break;
                }
            }

            if (m_hNotify)
            {
                ReceivedPacket* pkt = new ReceivedPacket();
                pkt->cmdId = header.cmdId;
                if (!bodyBuf.empty())
                    pkt->body = std::string(bodyBuf.begin(), bodyBuf.end());

                PostMessage(m_hNotify, WM_PACKET_RECEIVED,
                    static_cast<WPARAM>(header.cmdId),
                    reinterpret_cast<LPARAM>(pkt));
            }
        }
    }

    bool RecvExact(char* buf, uint32_t size)
    {
        uint32_t received = 0;
        while (received < size)
        {
            int ret = recv(m_socket,
                buf + received,
                static_cast<int>(size - received), 0);
            if (ret <= 0) return false;
            received += static_cast<uint32_t>(ret);
        }
        return true;
    }

    SOCKET              m_socket = INVALID_SOCKET;
    HWND                m_hNotify = nullptr;
    std::atomic<bool>   m_connected{ false };
    std::thread         m_recvThread;
    bool m_wsaOk = false;
};