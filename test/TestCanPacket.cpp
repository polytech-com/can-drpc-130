// Copyright 2022 - Polytech A/S
#include <catch2/catch_test_macros.hpp>

#include "CanPacket.h"

using namespace Drpc130;

TEST_CASE("Test CanPacket")
{
    static constexpr uint8_t frameLengthMin = 6;

    SECTION("Test SetDataRequest")
    {
        // Test data from chapter 3.2 in DRPC-130 Programmer Reference Manual PDF
        std::vector<uint8_t> data {0x00, 0x08, 0x00, 0x00, 0x07, 0x08, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};
        CanPacket frame(CanPacket::Command::SetDataRequest, data);
        CHECK(frame.valid());
        CHECK(frame.command() == CanPacket::Command::SetDataRequest);
        CHECK(frame.commandData() == data);
        CHECK(frame.length() == frameLengthMin + data.size());
    }

    SECTION("Test GetVersionResponse")
    {
        // Test data from chapter 3.7 in DRPC-130 Programmer Reference Manual PDF
        std::vector<uint8_t> data {0x01, 0x00};
        CanPacket frame(CanPacket::Command::GetVersionResponse, data);
        CHECK(frame.valid());
        CHECK(frame.command() == CanPacket::Command::GetVersionResponse);
        CHECK(frame.commandData() == data);
        CHECK(frame.length() == frameLengthMin + data.size());
    }

    SECTION("Test raw SetBaudRateResponse")
    {
        CanPacket frame;
        std::vector<uint8_t> rawData {0x24, 0x43, 0x39, 0x0e, 0x00, 0x47, 0x0a, 0x0d};
        for (const uint8_t data : rawData)
            frame.addData(data);
        CHECK(frame.valid());
        CHECK(frame.command() == CanPacket::Command::SetBaudRateResponse);
        CHECK(frame.length() == rawData.size());
    }

    SECTION("Test raw ReceiveData")
    {
        CanPacket frame;
        std::vector<uint8_t> rawData {0x24, 0x43, 0x36, 0x0e, 0x06, 0x00, 0x00, 0x00, 0x00, 0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0x00, 0x00, 0x97, 0x0a, 0x0d};
        for (const uint8_t data : rawData)
            frame.addData(data);
        CHECK(frame.valid());
        CHECK(frame.command() == CanPacket::Command::ReceiveData);
        CHECK(frame.length() == rawData.size());
    }
}
