// Copyright 2022 - Polytech A/S
#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <iostream>

#include <boost/crc.hpp>

namespace Drpc130 {

// See DRPC-130 Programmer Reference Manual PDF
class CanPacket {
public:
    CanPacket() = default;

    enum Command {
        SetBaudRateRequest = 0x42,
        SetBaudRateResponse = 0x39,
        SendDataRequest = 0x54,
        SendDataResponse = 0x38,
        GetVersionRequest = 0x56,
        GetVersionResponse = 0x40,
        SetDataRequest = 0x58,
        SetDataResponse = 0x33,
        ReceiveData = 0x36,
    };

    CanPacket(Command command, std::vector<uint8_t> &data)
    {
        setCommandData(command, data);
    }

    virtual ~CanPacket() = default;

    uint8_t command() const
    {
        if (m_buffer.size() > s_header.size())
            return m_buffer.at(s_header.size());

        return 0;
    }

    void setCommandData(Command command, std::vector<uint8_t> &data)
    {
        m_buffer.insert(m_buffer.end(), s_header.begin(), s_header.end());
        m_buffer.push_back(command);
        m_buffer.insert(m_buffer.end(), data.begin(), data.end());
        m_crc.process_bytes(m_buffer.data(), m_buffer.size());
        m_buffer.push_back(m_crc.checksum());
        m_buffer.insert(m_buffer.end(), s_stop.begin(), s_stop.end());
    }

    void addData(uint8_t data)
    {
        if (m_buffer.size() > s_frameLengthMax)
            m_buffer.clear();

        if (valid() || ((m_buffer.size() < s_header.size()) && (data != s_header.at(m_buffer.size()))))
            return;

        m_buffer.push_back(data);
    }

    std::vector<uint8_t> &data()
    {
        return m_buffer;
    }

    std::vector<uint8_t> commandData()
    {
        if (m_buffer.size() >= s_frameLengthMin)
            return std::vector<uint8_t>(m_buffer.begin() + s_header.size() + sizeof(command()), m_buffer.end() - sizeof(checksum()) - s_stop.size());

        return {};
    }

    bool valid()
    {
        if (command() && (m_buffer.size() == length())) {
            // Check that the last two bytes is equal to the stop bytes
            if (!memcmp(m_buffer.data() + m_buffer.size() - s_stop.size(), s_stop.data(), s_stop.size())) {
                m_crc.reset();
                m_crc.process_bytes(m_buffer.data(), m_buffer.size() - s_stop.size() - sizeof(checksum()));
                return m_buffer.at(m_buffer.size() - sizeof(checksum()) - s_stop.size()) == m_crc.checksum();
            }
        }

        return false;
    }

    uint8_t length() const
    {
        uint8_t frameLength = s_frameLengthMin;

        switch (command()) {
        case SetDataRequest:
        case ReceiveData:
            frameLength += 14;
            break;
        case GetVersionResponse:
        case SetBaudRateResponse:
            frameLength += 2;
            break;
        default:
            break;
        }

        return frameLength;
    }

    const uint8_t checksum()
    {
        return m_crc.checksum();
    }

    void clear()
    {
        m_buffer.clear();
    }

private:
    static constexpr uint8_t s_frameLengthMin = 6;
    static constexpr uint8_t s_frameLengthMax = 33 + s_frameLengthMin;
    static constexpr std::array<uint8_t, 2> s_header = { 0x24, 0x43 };
    static constexpr std::array<uint8_t, 2> s_stop = { 0x0A, 0x0D };

    boost::crc_optimal<8, 0x07, 0x00, 0x00> m_crc;
    std::vector<uint8_t> m_buffer;
};

class CanBaudRatePacket : public CanPacket
{
public:
    CanBaudRatePacket(uint32_t baudRate)
    {
        std::vector<uint8_t> data {0x00, 0x00};

        switch (baudRate)
        {
        case 50000:
            data[0] = 0x06;
            break;
        case 100000:
            data[0] = 0x0A;
            break;
        case 250000:
            data[0] = 0x0E;
            break;
        case 500000:
            data[0] = 0x11;
            break;
        case 1000000:
            data[0] = 0x14;
            break;
        default:
            std::cout << "Defaulting to a baud rate of 10000 bits/sec" << std::endl;
            break;
        }

        setCommandData(SetBaudRateRequest, data);
    }

    virtual ~CanBaudRatePacket() = default;
};

}