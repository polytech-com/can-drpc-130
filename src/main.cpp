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

    CanDataPacket packet;
    boost::asio::io_context ioContext { 2 };
    auto work = boost::asio::make_work_guard(ioContext);

    SerialInterface serial(ioContext, serialDevice, 115200);
    SocketCanInterface can(ioContext, canDevice);

    CanBaudRatePacket baudRatePacket(std::stoi(baudRate));
    serial.write(baudRatePacket.data());

    serial.read([&packet, &can, &serial](std::span<uint8_t> buffer) {
        printf("Received serial buffer with length %lu\n", buffer.size());
        for (const uint8_t data : buffer) {
            packet.addData(data);
            if (packet.valid()) {
                printf("Received serial packet with command %x\n", packet.command());
                switch (packet.command()) {
                case CanPacket::Command::ReceiveData: {
                    SocketCanFrame frame;
                    auto payload = packet.payload();
                    frame.header.extended_format(packet.extendedMode());
                    frame.header.id(packet.id());
                    frame.header.payload_length(payload.size());
                    std::copy(payload.begin(), payload.end(), frame.payload.begin());
                    printf("Sending packet on CAN interface\n");
                    can.write(frame);
                    break;
                }
                case CanPacket::Command::SetDataResponse: {
                    std::vector<uint8_t> data;
                    printf("Sending SendDataRequest\n");
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

    can.read([&serial](SocketCanFrame& frame) {
        printf("Received CAN packet with length %lu\n", frame.header.payload_length());
        CanDataPacket packet(frame.header.extended_format(), frame.header.payload_length(),
            frame.header.id(), frame.payload);
        printf("Sending SetDataRequest\n");
        serial.write(packet.data());
    });

    ioContext.run();
}
