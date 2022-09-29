// Copyright 2022 - Polytech A/S
#pragma once

#include <span>

#include <canary/frame_header.hpp>
#include <canary/interface_index.hpp>
#include <canary/raw.hpp>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace boost::asio;

struct SocketCanFrame
{
    canary::frame_header header;
    std::array<std::uint8_t, 8> payload;
};

using SocketCanReadCallback = std::function<void(SocketCanFrame&)>;

class SocketCanInterface {
public:
    SocketCanInterface(io_context &ioContext, std::string &deviceName)
        : m_socket(ioContext, canary::raw::endpoint(canary::get_interface_index(deviceName)))
    {
    }

    virtual ~SocketCanInterface() = default;

    void write(SocketCanFrame &frame)
    {
        m_socket.send(canary::net::buffer(&frame, sizeof(frame)));
    }

    void read(SocketCanReadCallback callback)
    {
        m_readCallback = callback;
        read();
    }

private:
    void readCallback(const boost::system::error_code& error)
    {
        if (!error) {
            if (m_readCallback)
                m_readCallback(m_frame);

            read();
        }
    }

    void read()
    {
        m_socket.async_receive(canary::net::buffer(&m_frame, sizeof(m_frame)), boost::bind(&SocketCanInterface::readCallback, this, 
            boost::asio::placeholders::error));
    }

    SocketCanFrame m_frame;
    canary::raw::socket m_socket;
    SocketCanReadCallback m_readCallback;
};
