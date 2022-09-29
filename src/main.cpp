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
    desc.add_options()("can,c", boost::program_options::value(&canDevice)->default_value("vcan0"), "Virtual CAN device")("baud,b", boost::program_options::value(&baudRate)->default_value("500000"), "CAN baud rate in bit/s")("serial,s", boost::program_options::value(&serialDevice)->default_value("ttyACM0"), "Serial CAN device")("help,h", "Help message");
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc).run(), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cerr << desc;
        return 1;
    }

    CanPacket packet;
    boost::asio::io_context ioContext;
    boost::asio::make_work_guard(ioContext);

    SerialInterface serial(ioContext, serialDevice, 115200);
    SocketCanInterface can(ioContext, canDevice);

    CanBaudRatePacket baudRatePacket(std::stoi(baudRate));
    serial.write(baudRatePacket.data());

    serial.read([&packet, &can](std::span<uint8_t> buffer) {
        printf("Received serial buffer with length %lu\n", buffer.size());
        for (const uint8_t data : buffer) {
            packet.addData(data);
            if (packet.valid()) {
                printf("Received serial packet with command %d\n", packet.command());
                if (packet.command() == CanPacket::Command::ReceiveData) {
                    SocketCanFrame frame;
                    CanDataPacket& dataPacket = dynamic_cast<CanDataPacket&>(packet);
                    auto payload = dataPacket.payload();
                    frame.header.extended_format(dataPacket.extendedMode());
                    frame.header.id(dataPacket.id());
                    frame.header.payload_length(payload.size());
                    std::copy(payload.begin(), payload.end(), frame.payload.begin());
                    printf("Sending packet on CAN interface\n");
                    can.write(frame);
                }
            }
        }
    });

    can.read([&serial](SocketCanFrame& frame) {
        printf("Received CAN packet with length %lu\n", frame.header.payload_length());
        CanDataPacket packet(frame.header.extended_format(), frame.header.payload_length(),
            frame.header.id(), frame.payload);
        if (packet.valid()) {
            printf("Sending packet on serial interface\n");
            serial.write(packet.data());
        }
    });

    ioContext.run();
}
