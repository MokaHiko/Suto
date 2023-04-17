#ifndef SERVER_H
#define SERVER_H

#pragma once

#include <memory>

#include "common.h"
#include "tsqueue.h"
#include "message.h"
#include "connection.h"

namespace sto
{
    template <typename T>
    class server
    {
    public:
        server(uint16_t port)
            : m_asioAcceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port))
        {
        }

        ~server()
        {
            stop();
        }

        bool start()
        {
            try
            {
                waitForClientConnection();
                m_thrContext = std::thread([this]()
                                           { m_context.run(); });
            }
            catch (std::exception &e)
            {

                std::cerr << e.what() << std::endl;
                return false;
            }

            printf("[SERVER] STARTED!\n");
            return true;
        }

        void stop()
        {
            // try stop asio context
            m_context.stop();

            // wait on thread to be joinable
            if (m_thrContext.joinable())
                m_thrContext.join();

            printf("[SERVER] STOPPED!\n");
        }

        // Server handles msg queues by calling this step function
        void update(size_t nMaxMessages = -1, bool passiveRun = false)
        {
            // Enabled on passive run
            if(passiveRun)
                m_qMessagesIn.wait();

            size_t nMessageCount = 0;
            while(nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
            {
                owned_message<T> msg = m_qMessagesIn.pop_front();
                OnMessage(msg.remote, msg.msg);

                nMessageCount++;
            }
        }

    protected:
        void waitForClientConnection()
        {
            m_asioAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket)
                                        {
                if(!ec)
                {
                    std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";

                    std::shared_ptr<connection<T>> newconn = std::make_shared<connection<T>>(
                        connection<T>::owner::server,   // Assign owner of connection to be a server
                        m_context,                      // Give context handle
                        std::move(socket),              // Move socket to connection
                        m_qMessagesIn);                 // Give handle to q of messages in
                
                    if(onClientConnect(newconn))
                    {
                        m_connections.push_back(newconn);
                        m_connections.back()->connectToClient(m_idCounter++);

                        std::cout << "[SERVER]: Client Connection Approved with ID: " << m_connections.back()->getId() << " Approved Access!\n";
                    }
                    else
                    {
                        std::cout << "[SERVER]: " << socket.remote_endpoint() << " Denied Access!\n";
                    }
                }
                else
                {
                    std::cout << "[SERVER] New Connection Error: " << ec.value() << "\n";
                }

                waitForClientConnection(); });
        }

        void messageClient(std::shared_ptr<connection<T>> client, const message<T> &msg)
        {
            if(client && client->isConnected())
            {
                client->send(msg);
            }
            else
            {
                // TODO: Handle client reconnections
                onClientDisconnect(client);
                client.reset();

                // TODO: Erase Remove
                m_connections.erase(std::remove(m_connection.begin(), m_connection.end(), client), m_connection.end());
            }
        }

        void messageAllClients(const message<T> &msg, std::shared_ptr<connection<T>> ignoredClient)
        {
            bool hasDisconnects = false;

            for(auto& client: m_connections)
            {
                if(client && client->isConnected())
                {
                    if(client != ignoredClient)
                        client->send(msg);
                }
                else
                {
                    onClientDisconnect(client);
                    client.reset();

                    hasDisconnects = true;
                }
            }

            if(hasDisconnects)
            {
                m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), nullptr), m_connections.end());
            }
        }

    protected:
        // ASYNC
        // Callback that returns whether a user can/is allowed connect
        virtual bool onClientConnect(std::shared_ptr<connection<T>> client)
        {
            return false;
        }

        // ASYNC
        // Callback called when user disconnected
        virtual void onClientDisconnect(std::shared_ptr<connection<T>> client)
        {
        }

        // Callback called each Update() call to handle client messages
        virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
        {

        }

    protected:
        // Thread safe queue for incoming message packets
        tsqueue<owned_message<T>> m_qMessagesIn;

        asio::io_context m_context;
        std::thread m_thrContext;

        // Asio tcp connection acceptor
        asio::ip::tcp::acceptor m_asioAcceptor;

        std::deque<std::shared_ptr<connection<T>>> m_connections;

        uint32_t m_idCounter = 0;
    };
}

#endif