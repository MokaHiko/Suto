#ifndef CONNECTION_H
#define CONNECTION_H

#pragma once

#include <memory>

#include "common.h"
#include "message.h"
#include "tsqueue.h"

namespace sto
{
    // Main interface of client and server.
    // Owns the socket of each client-server connection, and holds message queues
    template <typename T>
    class connection : public std::enable_shared_from_this<connection<T>>
    {
    public:
        enum class owner
        {
            server,
            client
        };

        connection(owner parent, asio::io_context &context, asio::ip::tcp::socket socket, tsqueue<owned_message<T>> &qIn)
            :m_sharedContext(context), m_socket(std::move(socket)), m_qMessagesIn(qIn)
        {
            m_owner = parent;
        };

        ~connection(){};

    public:
        uint32_t getId() { return m_connectionId; }

        // Called by server to establish connnection to client and register Id
        void connectToClient(uint32_t id)
        {
            if (m_owner == owner::server)
            {
                if (m_socket.is_open())
                {
                    m_connectionId = id;
                    readHeader();
                }
            }
        }

        // Called by clients to establish connnection to server
        bool connectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
        {
            if (m_owner == owner::client)
            {
                // Connect socket to endpoint
                asio::async_connect(m_socket, endpoints, [this](std::error_code ec, asio::ip::tcp::endpoint endpoint){
                    if(!ec)
                    {
                        // Prime asio context to read header
                        readHeader();
                    }
                    else
                    {
                        std::cout << "[Client]: " << "failed to connect to server!\n";
                    }
                });
                return true;
            }

            return false;
        };

        void disconnect()
        {
            if(isConnected())
            {
                asio::post(m_sharedContext, [this](){
                    m_socket.close();
                });
            }
        };
        bool isConnected() const
        {
            return m_socket.is_open();
        };

    public:
        void send(const message<T> &msg)
        {
            asio::post(m_sharedContext, [this, msg](){

                // Only add a queue to write messages if not already writing
                bool isWriting = !m_qMessagesOut.empty();
                m_qMessagesOut.push_back(msg);

                if(!isWriting)
                {
                    writeHeader();
                }
            });
        };

    private:
        // Async
        // Read message header from connection socket to buffer body
        void readHeader()
        {
            asio::async_read(m_socket, asio::buffer(&m_messageBuffer.header, sizeof(message_header<T>)), [this](std::error_code ec, std::size_t length)
                             {
                if(!ec)
                {
                    if(m_messageBuffer.header.size > 0)
                    {
                        m_messageBuffer.body.resize(m_messageBuffer.header.size);
                        readBody();
                    }
                    else
                    {
                        addToIncomingMessageQueue();
                    }
                }
                else
                {
                    std::cout << "Client: [" << m_connectionId << "]" << " Read Header Fail!\n";
                    m_socket.close();
                } });
        }

        // ASYNC
        // Read message body from connection socket to buffer body
        void readBody()
        {
            // Message buffer already resized when reading header
            asio::async_read(m_socket, asio::buffer(m_messageBuffer.body.data(), m_messageBuffer.size()), [this](std::error_code ec, std::size_t length)
                             {
                    if(!ec)
                    {
                        addToIncomingMessageQueue();
                    }
                    else
                    {
                        std::cout << "Client: [" << m_connectionId << "]" << " Read  Body Fail!\n";
                        m_socket.close();
                    }});
        }

        // Async
        // Write(Send) message header from front of send message queue
        void writeHeader()
        {
            asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)), [this](std::error_code ec, size_t length)
                              {
                if(!ec)
                {
                    if(m_qMessagesOut.front().header.size > 0)
                    {
                        writeBody();
                    }
                    else
                    {
                        m_qMessagesOut.pop_front();

                        if(!m_qMessagesOut.empty())
                            writeHeader();
                    }
                } 
                else
                {
                    std::cout << "[" << m_connectionId << "]" << " Write Header Fail!\n";
                    m_socket.close();
                }
                });
        }

        // Async
        // Write(Send) message body from front of send message queue
        void writeBody()
        {
            asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()), [this](std::error_code ec, size_t t){
                if(!ec)
                {
                    // Pop message from write queue
                    m_qMessagesOut.pop_front();
                    if(!m_qMessagesOut.empty())
                        writeHeader();
                }
                else
                {
                    std::cout << "[" << m_connectionId << "]" << " Write Body Fail!\n";
                    m_socket.close();
                }
            });
        }

        void addToIncomingMessageQueue()
        {
            // If owner of connection is server, transform to messageBuffer to owned message then push on queue
            if (m_owner == owner::server)
                m_qMessagesIn.push_back({this->shared_from_this(), m_messageBuffer});
            else
                m_qMessagesIn.push_back({nullptr, m_messageBuffer});

            // this function is called after every async read, so queue read again
            readHeader();
        }

    protected:
        // Holds all queue of messages to be sent to remote stored in socket
        tsqueue<message<T>> m_qMessagesOut;

        // Reference to queue owned by server
        // Only servers require owned messages
        tsqueue<owned_message<T>> &m_qMessagesIn;

        // Incoming messages will be stored temporarily in this buffer
        message<T> m_messageBuffer;

    private:
        uint32_t m_connectionId;
        owner m_owner = owner::server;

        asio::ip::tcp::socket m_socket;
        asio::io_context &m_sharedContext;
    };
}

#endif