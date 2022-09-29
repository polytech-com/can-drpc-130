// Copyright 2022 - Polytech A/S
#include <catch2/catch_test_macros.hpp>

#include "CanPacket.h"

using namespace Drpc130;

TEST_CASE("Test CanPacket")
{
    static constexpr uint8_t packetLengthMin = 6;

    SECTION("Test SetDataRequest")
    {
        // Test data from chapter 3.2 in DRPC-130 Programmer Reference Manual PDF
        std::vector<uint8_t> data {0x00, 0x08, 0x00, 0x00, 0x07, 0x08, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};
        CanPacket packet(CanPacket::Command::SetDataRequest, data);
        CHECK(packet.valid());
        CHECK(packet.command() == CanPacket::Command::SetDataRequest);
        CHECK(packet.commandData() == data);
        CHECK(packet.length() == packetLengthMin + data.size());
    }

    SECTION("Test GetVersionResponse")
    {
        // Test data from chapter 3.7 in DRPC-130 Programmer Reference Manual PDF
        std::vector<uint8_t> data {0x01, 0x00};
        CanPacket packet(CanPacket::Command::GetVersionResponse, data);
        CHECK(packet.valid());
        CHECK(packet.command() == CanPacket::Command::GetVersionResponse);
        CHECK(packet.commandData() == data);
        CHECK(packet.length() == packetLengthMin + data.size());
    }

    SECTION("Test raw SetBaudRateResponse")
    {
        CanPacket packet;
        std::vector<uint8_t> rawData {0x24, 0x43, 0x39, 0x0e, 0x00, 0x47, 0x0a, 0x0d};
        for (const uint8_t data : rawData)
            packet.addData(data);
        CHECK(packet.valid());
        CHECK(packet.command() == CanPacket::Command::SetBaudRateResponse);
        CHECK(packet.length() == rawData.size());
    }

    SECTION("Test raw ReceiveData")
    {
        CanPacket packet;
        std::vector<uint8_t> rawData {0x24, 0x43, 0x36, 0x0e, 0x06, 0x00, 0x00, 0x00, 0x00, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0x00, 0x00, 0x97, 0x0a, 0x0d};
        for (const uint8_t data : rawData)
            packet.addData(data);
        CHECK(packet.valid());
        CHECK(packet.command() == CanPacket::Command::ReceiveData);
        CHECK(packet.length() == rawData.size());
    }
}

TEST_CASE("Test CanDataPacket")
{
    uint32_t id = 0x01020304;
    bool extendedMode = true;
    static constexpr uint8_t length = 8;
    std::array<uint8_t, length> payload = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

    CanDataPacket packet(extendedMode, length, id, payload);
    CHECK(packet.valid());
    CHECK(packet.command() == CanPacket::Command::SetDataRequest);
    CHECK(packet.commandData().at(0) == 0);
    CHECK(packet.commandData().at(1) == ((extendedMode << 7) | length));
    CHECK_FALSE(memcmp(packet.commandData().data() + 2, &id, sizeof(id)));
    CHECK_FALSE(memcmp(packet.commandData().data() + 6, &payload, sizeof(payload)));
}
