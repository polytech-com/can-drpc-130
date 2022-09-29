// Copyright 2022 - Polytech A/S
#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

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

    CanPacket(Command command, std::vector<uint8_t>& data)
    {
        setCommandData(command, data);
    }

    virtual ~CanPacket() = default;

    uint8_t command() const
    {
        if (m_data.size() > s_header.size())
            return m_data.at(s_header.size());

        return 0;
    }

    void setCommandData(Command cmd, std::vector<uint8_t>& data)
    {
        m_data.insert(m_data.end(), s_header.begin(), s_header.end());
        m_data.push_back(cmd);
        m_data.insert(m_data.end(), data.begin(), data.end());
        m_crc.process_bytes(m_data.data(), m_data.size());
        m_data.push_back(m_crc.checksum());
        m_data.insert(m_data.end(), s_stop.begin(), s_stop.end());
    }

    std::vector<uint8_t> commandData()
    {
        return m_commandData;
    }

    void addData(uint8_t data)
    {
        if (m_data.size() > s_packetLengthMax)
            m_data.clear();

        if (valid() || ((m_data.size() < s_header.size()) && (data != s_header.at(m_data.size()))))
            return;

        m_data.push_back(data);
    }

    std::vector<uint8_t>& data()
    {
        return m_data;
    }

    bool valid()
    {
        if (command() && (m_data.size() == length())) {
            m_commandData.assign(m_data.begin() + s_header.size() + sizeof(command()), m_data.end() - sizeof(checksum()) - s_stop.size());

            // Check that the last two bytes is equal to the stop bytes
            if (!memcmp(m_data.data() + m_data.size() - s_stop.size(), s_stop.data(), s_stop.size())) {
                m_crc.reset();
                m_crc.process_bytes(m_data.data(), m_data.size() - s_stop.size() - sizeof(checksum()));
                return m_data.at(m_data.size() - sizeof(checksum()) - s_stop.size()) == m_crc.checksum();
            }
        }

        return false;
    }

    uint8_t length() const
    {
        uint8_t packetLength = s_packetLengthMin;

        switch (command()) {
        case SetDataRequest:
        case ReceiveData:
            packetLength += 14;
            break;
        case GetVersionResponse:
        case SetBaudRateRequest:
        case SetBaudRateResponse:
            packetLength += 2;
            break;
        default:
            break;
        }

        return packetLength;
    }

    const uint8_t checksum()
    {
        return m_crc.checksum();
    }

    void clear()
    {
        m_data.clear();
    }

protected:
    static constexpr uint8_t s_packetLengthMin = 6;
    static constexpr uint8_t s_packetLengthMax = 33 + s_packetLengthMin;
    static constexpr std::array<uint8_t, 2> s_header = { 0x24, 0x43 };
    static constexpr std::array<uint8_t, 2> s_stop = { 0x0A, 0x0D };

    std::vector<uint8_t> m_data;
    std::vector<uint8_t> m_commandData;
    boost::crc_optimal<8, 0x07, 0x00, 0x00> m_crc;
};

class CanBaudRatePacket : public CanPacket {
public:
    CanBaudRatePacket(uint32_t baudRate)
    {
        std::vector<uint8_t> data { 0x00, 0x00 };

        switch (baudRate) {
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

class CanDataPacket : public CanPacket {
public:
    CanDataPacket(bool extendedMode, uint8_t length, uint32_t id, std::array<uint8_t, 8>& payload)
    {
        std::vector<uint8_t> data;
        std::array<uint8_t, 4> idArray;
        memcpy(idArray.data(), &id, idArray.size());

        data.push_back(0);
        data.push_back((extendedMode << 7) | length);
        data.insert(data.end(), idArray.begin(), idArray.end());
        data.insert(data.end(), payload.begin(), payload.begin() + length);

        setCommandData(SetDataRequest, data);
    }

    bool extendedMode()
    {
        return (m_commandData.at(1) >> 7) & 0x1;
    }

    uint32_t id()
    {
        uint32_t value;
        memcpy(&value, m_commandData.data() + 2, sizeof(value));

        return value;
    }

    std::vector<uint8_t> payload()
    {
        return std::vector<uint8_t>(m_commandData.begin() + 6, m_commandData.end());
    }

    virtual ~CanDataPacket() = default;
};

} // namespace Drpc130