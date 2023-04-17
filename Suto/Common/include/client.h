#ifndef CLIENT_H
#define CLIENT_H

#pragma once

#include "common.h"
#include "message.h"
#include "tsqueue.h"
#include "connection.h"

namespace sto
{
    // Client handles msg queues by popping msgs from incoming tsqueue
    template <typename T>
    class client
    {
    public:
        client()
            : m_socket(m_context)
            {

            };

        ~client()
        {
            disconnect();
        };

    public:
        bool connect(const std::string &host, const uint16_t port)
        {
            try
            {
                // Resolve ip address and get first endpoint
                asio::ip::tcp::resolver resolver(m_context);
                auto endpoints = resolver.resolve(host, std::to_string(port));
            

                // Create connection
                m_connection = std::make_unique<connection<T>>(
                    connection<T>::owner::client,
                    m_context,
                    asio::ip::tcp::socket(m_context),
                    m_qMessagesIn);

                // tell connection to connect to endpoint
                m_connection->connectToServer(endpoints);

                // start asio thread context
                m_thrContext = std::thread([this]()
                                           { m_context.run(); });
            }
            catch (std::exception &e)
            {
                std::cerr << e.what() << std::endl;
                return false;
            }

            return true;
        }

        bool isConnected() const
        {
            if (m_connection)
                return m_connection->isConnected();

            return false;
        }

        void disconnect()
        {
            if (!isConnected())
            {
                m_connection->disconnect();
            }

            m_context.stop();

            if (m_thrContext.joinable())
                m_thrContext.join();

            m_connection.release();
        }

    public:
        void send(const message<T> &msg)
        {
            m_connection->send(msg);
        }

    public:
        tsqueue<owned_message<T>> &incomingMessages()
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