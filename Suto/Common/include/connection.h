#ifndef CONNECTION_H
#define CONNECTION_H

#pragma once

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
        connection(asio::io_context &context)
            : m_shared_context(context)
        {

        };
        ~connection(){};

    public:
        bool ConnectToServer(const asio::ip::tcp::endpoint &endpoint)
        {
            return false;
        };
        bool Disconnect()
        {
            return false;
        };
        bool IsConnected() const
        {
            return false;
        };

    public:
        void Send(const message<T> &msg)
        {

        };

    protected:
        // Holds all message to be sent to remote
        tsqueue<message<T>> m_qMessagesOut;

        // Reference to queue owned by server/client
        tsqueue<owned_message<T>> &m_qMessagesIn;

    private:
        asio::ip::tcp::socket m_socket;
        asio::io_context &m_shared_context;
    };
}

#endif