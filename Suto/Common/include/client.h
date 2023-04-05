#ifndef CLIENT_H
#define CLIENT_H

#pragma once

#include "common.h"
#include "message.h"
#include "tsqueue.h"
#include "connection.h"

namespace sto
{
    template<typename T>
    class client
    {
    public:
        client()
            :m_socket(m_context)
        {

        };
        ~client()
        {
            Disconnect();
        };
    public:

        bool Connect(const std::string& host, const uint16_t port)
        {
            try
            {
                // Create connection
                m_connection = std::make_unique<connection<T>>();

                // Resolve ip address and get first endpoint
                asio::ip::tcp::resolver resolver(m_context);
                asio::ip::tcp::endpoint endpoint = resolver.resolve(host, std::to_string(port)).begin();

                // tell connection to connect to endpoint
                m_connection->ConnectToServer(endpoint);

                // start asio thread context
                m_thrContext = std::thread([this](){m_context.run();});
            }
            catch(std::exception& e)
            {
                std::cerr << e.what() << std::endl;
                return false;
            }

            return true;
        }

        bool IsConnected() const
        {
            if(m_connection)
                return m_connection->IsConnected();

            return false;
        }

        void Disconnect()
        {
            if(!IsConnected())
            {
                m_connection->Disconnect();
            }

            m_context.stop();

            if(m_thrContext.joinable())
                m_thrContext.join();
            
            m_connection.release();
        }

    public:
        void Send(const msg& msg)
        {
            m_connection->Send(msg);
        }
    public:
        tsqueue<owned_message<T>>& IncomingMessages()
        {
            return m_qMessagesIn;
        }

    private:
        // Thread safe queue of owned messages coming from the server
        tsqueue<owned_message<T>> m_qMessagesIn;


    private:
        asio::io_context m_context;
        std::thread m_thrContext;

        // Socket used to connect to server then moved to connection
        asio::ip::tcp::socket m_socket;

        // Client has single connection instance which handles data transfer
        std::unique_ptr<connection<T>> m_connection;
    };
}
#endif