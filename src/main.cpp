// Copyright 2022 - Polytech A/S
#include <string>
#include <iostream>

#include <boost/program_options.hpp>

#include "CanPacket.h"
#include "SerialInterface.h"
#include "SocketCanInterface.h"

using namespace Drpc130;

int main(int argc, char** argv) {
    std::string baudRate;
    std::string canDevice;
    std::string serialDevice;

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("can,c", boost::program_options::value(&canDevice)->default_value("vcan0"), "Virtual CAN device")
        ("baud,b", boost::program_options::value(&baudRate)->default_value("500000"), "CAN baud rate in bit/s")
        ("serial,s", boost::program_options::value(&serialDevice)->default_value("ttyACM0"), "Serial CAN device")
        ("help,h", "Help message");
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options (desc).run (), vm);
    boost::program_options::notify(vm);

    if (vm.count ("help")) {
        std::cerr << desc;
        return 1;
    }

    CanPacket packet;
    boost::asio::io_context ioContext;
    auto work = boost::asio::make_work_guard(ioContext);

    SerialInterface serial(ioContext, serialDevice, 115200);
    SocketCanInterface can(ioContext, canDevice);

    CanBaudRatePacket baudRatePacket(std::stoi(baudRate));
    serial.write(baudRatePacket.buffer());

    serial.read([&packet, &can](std::span<uint8_t> buffer) {
        for (const uint8_t data : buffer) {
            packet.addData(data);
            if (packet.valid()) {
                printf("Received packet with command %d\n", packet.command());
                // FIXME: Handle packet
            }
        }
    });

    ioContext.run();
}
