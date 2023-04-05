#ifndef SERVERUDP_H
#define SERVERUDP_H

#pragma once

#include "Common.h"

class ServerUDP
{
public:
    ServerUDP();
    ~ServerUDP();

    void Run();
private:
    void StartReceive();
    void HandleReceive(const asio::error_code& e, size_t length);

    void HandleSend(const asio::error_code& e, size_t length);
private:
    asio::io_context m_Context = {};
    asio::ip::udp::socket m_Socket;
    asio::ip::udp::endpoint m_RemoteEndpoint;

    std::array<char, 1> m_RecieveBuffer;
    std::array<char, 1000> m_SendBuffer;
};



#endif