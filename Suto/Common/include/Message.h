#ifndef MESSAGE_H
#define MESSAGE_H

#pragma once

#include "common.h"

namespace sto
{
    // Headers will be templated by enum class data type T
    // Headers will contain the enum type {Id} and Body Size {Size}
    template <typename T>
    struct message_header
    {
        T id{};

        // Returns size of entire message packet sizeof(Header + Body)
        uint32_t size = 0;
    };

    template <typename T>
    struct message
    {
        message_header<T> header{};
        std::vector<uint8_t> body;

        // Returns size of entire message packet sizeof(header + body)
        size_t size() const
        {
            return sizeof(message_header<T>) + body.size();
        }

        // Produces friendly description of message
        friend std::ostream &operator<<(std::ostream &os, const message<T> &msg)
        {
            os << "id: " << msg.header.id << " size: " << msg.size();
            return os;
        }

        // pushes data of type datatype to message body buffer
        template <typename DataType>
        friend message<T> &operator<<(message<T> &msg, DataType &data)
        {
            static_assert(std::is_standard_layout<DataType>::value, "data is too complex to be stored!");

            // resize body buffer to store new data
            size_t previoussize = msg.body.size();
            msg.body.resize(previoussize + sizeof(DataType));

            // copy data to body buffer offsetted by previous size
            memcpy(msg.body.data() + previoussize, &data, sizeof(DataType));

            // update header
            msg.header.size = msg.size();

            return msg;
        }

        // pop data of type datatype out of message body buffer
        template <typename DataType>
        friend message<T> &operator>>(message<T> &msg, DataType &data)
        {
            static_assert(std::is_standard_layout<DataType>::value, "data is too complex to be extracted!");

            // copy data to data type
            size_t newsize = msg.body.size() - sizeof(DataType);
            memcpy(&data, msg.body.data() + newsize, sizeof(DataType));

            // resize buffer and update header
            msg.body.resize(newsize);
            msg.header.size = msg.size();

            return msg;
        }
    };

    class connection;

    // Msg wrapper that has handle on connection
    template<typename T>
    struct owned_message
    {
        std::shared_ptr<connection<T>> remote = nullptr;
        message<T> msg;

        friend std::ostream& operator<<(std::ostream& os, owned_message& msg)
        {
            os << msg.msg;
            return os;
        }
    };
}

#endif