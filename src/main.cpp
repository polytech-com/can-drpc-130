// Copyright 2022 - Polytech A/S
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "CanPacket.h"
#include "SerialInterface.h"
#include "SocketCanInterface.h"

using namespace Drpc130;

int main(int argc, char** argv)
{
    std::string baudRate;
    std::string canDevice;
    std::string serialDevice;

    boost::program_options::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()("can,c", boost::program_options::value(&canDevice)->default_value("vcan0"),
        "Virtual CAN device")("baud,b", boost::program_options::value(&baudRate)->default_value("500000"),
        "CAN baud rate in bit/s")("serial,s", boost::program_options::value(&serialDevice)->default_value("ttyACM0"),
        "Serial CAN device")("help,h", "Help message");
    // clang-format on
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cerr << desc;
        return 1;
    }

    CanDataPacket packet;
    // Use a single io_context for now to avoid using a strand for thread synchronization
    boost::asio::io_context ioContext;
    auto work = boost::asio::make_work_guard(ioContext);

    SerialInterface serial(ioContext, serialDevice, 115200);
    SocketCanInterface can(ioContext, canDevice);

    // Set the CAN baud rate from the specified input
    CanBaudRatePacket baudRatePacket(std::stoi(baudRate));
    serial.write(baudRatePacket.data());

    // Start the async read on the serial interface
    serial.read([&packet, &can, &serial](std::span<uint8_t> buffer) {
        for (const uint8_t data : buffer) {
            packet.addData(data);
            if (packet.valid()) {
                switch (packet.command()) {
                case CanPacket::Command::ReceiveData: {
                    SocketCanFrame frame;
                    auto payload = packet.payload();
                    frame.header.extended_format(packet.extendedMode());
                    frame.header.id(packet.id());
                    frame.header.payload_length(payload.size());
                    std::copy(payload.begin(), payload.end(), frame.payload.begin());
                    can.write(frame);
                    break;
                }
                case CanPacket::Command::SetDataResponse: {
                    std::vector<uint8_t> data;
                    CanPacket sendDataPacket(CanPacket::Command::SendDataRequest, data);
                    serial.write(sendDataPacket.data());
                    break;
                }
                default:
                    break;
                }

                packet.clear();
            }
        }
    });

    // Start the async read on the CAN interface
    can.read([&serial](SocketCanFrame& frame) {
        CanDataPacket packet(frame.header.extended_format(), frame.header.payload_length(),
            frame.header.id(), frame.payload);
        serial.write(packet.data());
    });

    ioContext.run();
    return 0;
}
