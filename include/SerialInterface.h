// Copyright 2022 - Polytech A/S
#pragma once

#include <span>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

using namespace boost::asio;

using SerialReadCallback = std::function<void(std::span<uint8_t>)>;

class SerialInterface {
public:
    SerialInterface(io_context& ioContext, std::string& deviceName, uint32_t baudRate)
        : m_serial(ioContext, deviceName)
    {
        m_serial.set_option(serial_port_base::baud_rate(baudRate));
        m_serial.set_option(serial_port_base::character_size(8));
        m_serial.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
        m_serial.set_option(serial_port_base::parity(serial_port_base::parity::none));
        m_serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    }

    virtual ~SerialInterface() = default;

    void write(std::vector<uint8_t>& data)
    {
        boost::asio::write(m_serial, boost::asio::buffer(data.data(), data.size()));
    }

    void read(SerialReadCallback callback)
    {
        m_readCallback = callback;
        read();
    }

private:
    void readCallback(std::size_t bytes)
    {
        if (m_readCallback)
            m_readCallback({ m_buffer.data(), bytes });

        read();
    }

    void read()
    {
        m_serial.async_read_some(boost::asio::buffer(m_buffer), boost::bind(&SerialInterface::readCallback, this, boost::asio::placeholders::bytes_transferred));
    }

    serial_port m_serial;
    std::array<uint8_t, 64> m_buffer;
    SerialReadCallback m_readCallback;
};
