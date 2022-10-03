// Copyright 2022 - Polytech A/S
#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <boost/crc.hpp>

namespace Drpc130 {

// See DRPC-130 Programmer Reference Manual PDF
class CanPacket {
public:
    CanPacket() = default;

    enum Command : uint8_t {
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

    /// @brief Constructor
    /// @param command The command to create
    /// @param commandData A reference to the data for the command
    CanPacket(Command command, const std::vector<uint8_t>& commandData)
    {
        setCommandData(command, commandData);
    }

    virtual ~CanPacket() = default;

    /// @brief Returns the command
    /// @return An uint8_t with the command
    uint8_t command() const
    {
        if (m_data.size() > s_header.size())
            return m_data.at(s_header.size());

        return 0;
    }

    /// @brief Sets the command data (alternative to constructor with arguments)
    /// @param command The command to create
    /// @param commandData A reference to the data for the command
    void setCommandData(Command cmd, const std::vector<uint8_t>& commandData)
    {
        m_data.insert(m_data.end(), s_header.begin(), s_header.end());
        m_data.emplace_back(cmd);
        m_data.insert(m_data.end(), commandData.begin(), commandData.end());
        m_crc.process_bytes(m_data.data(), m_data.size());
        m_data.emplace_back(m_crc.checksum());
        m_data.insert(m_data.end(), s_stop.begin(), s_stop.end());
    }

    /// @brief Returns the command data part of the packet
    /// @return An std::vector with the command data
    std::vector<uint8_t> commandData()
    {
        return m_commandData;
    }

    /// @brief Adds a data byte to be decoded based on the MCU protocol
    /// @param data An uint8_t to be added
    void addData(uint8_t data)
    {
        if (m_data.size() > s_packetLengthMax)
            m_data.clear();

        if (valid() || ((m_data.size() < s_header.size()) && (data != s_header.at(m_data.size()))))
            return;

        m_data.emplace_back(data);
    }

    /// @brief Returns the data
    /// @return An std::vector reference with the data
    std::vector<uint8_t>& data()
    {
        return m_data;
    }

    /// @brief Checks if the packet is valid. Can be used to find out when a complete packet is ready
    /// @return A bool telling if the packet is valid
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

    /// @brief Returns the length of the packet
    /// @return An uint8_t with the length
    uint8_t length() const
    {
        uint8_t packetLength { s_packetLengthMin };

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

    /// @brief Returns the checksum of the packet (mainly used for testing)
    /// @return An uint8_t with the checksum
    uint8_t checksum()
    {
        return m_crc.checksum();
    }

    /// @brief Clears the packet data
    void clear()
    {
        m_data.clear();
        m_commandData.clear();
    }

protected:
    static constexpr uint8_t s_packetLengthMin { 6 };
    static constexpr uint8_t s_packetLengthMax { 33 + s_packetLengthMin };
    static constexpr std::array<uint8_t, 2> s_header = { 0x24, 0x43 };
    static constexpr std::array<uint8_t, 2> s_stop = { 0x0A, 0x0D };

    std::vector<uint8_t> m_data;
    std::vector<uint8_t> m_commandData;
    boost::crc_optimal<8, 0x07, 0x00, 0x00> m_crc;
};

class CanBaudRatePacket : public CanPacket {
public:
    /// @brief Constructor
    /// @param baudRate The baud rate in bit/s
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
            break;
        }

        setCommandData(SetBaudRateRequest, data);
    }

    virtual ~CanBaudRatePacket() = default;
};

class CanDataPacket : public CanPacket {
public:
    CanDataPacket() = default;

    /// @brief Constructor
    /// @param extendedMode Use to select between normal or extended mode
    /// @param payloadLength The length of the payload
    /// @param id The frame ID to be used
    /// @param payload The payload to be sent
    CanDataPacket(bool extendedMode, uint8_t payloadLength, uint32_t id, const std::array<uint8_t, 8>& payload)
    {
        std::vector<uint8_t> data;
        std::array<uint8_t, 4> idArray;
        memcpy(idArray.data(), &id, idArray.size());

        data.emplace_back(0);
        data.emplace_back((extendedMode << 7) | payloadLength);
        data.insert(data.end(), idArray.begin(), idArray.end());
        data.insert(data.end(), payload.begin(), payload.begin() + payloadLength);

        setCommandData(SetDataRequest, data);
    }

    virtual ~CanDataPacket() = default;

    /// @brief Returns the extended mode bit
    /// @return A bool with the result
    bool extendedMode()
    {
        return (m_commandData.at(1) >> 7) & 0x1;
    }

    /// @brief Returns the frame ID
    /// @return An uint32_t with the result
    uint32_t id()
    {
        uint32_t value;
        memcpy(&value, m_commandData.data() + 2, sizeof(value));

        return value;
    }

    /// @brief Returns the payload from the packet
    /// @return A std::vector with the result
    const std::vector<uint8_t> payload()
    {
        return std::vector<uint8_t>(m_commandData.begin() + 2 + sizeof(id()), m_commandData.end());
    }
};

} // namespace Drpc130
