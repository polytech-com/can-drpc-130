# can-drpc-130

The `can-drpc-130` tool can be used to bridge a virtual CAN device with the TTY device of the CAN serial port on a [IEI DRPC-130](https://www.ieiworld.com/en/product/model.php?II=598). It implements the MCU protocol needed for sending and receiving CAN packets from the dual CAN D-Sub9 female connector.

For the tool to work the following kernel options are needed:

```
CONFIG_USB_ACM=y
CONFIG_CAN=y
CONFIG_CAN_RAW=y
CONFIG_CAN_VCAN=y
```

The tool can then be started by the following commands:

```
ip link add dev vcan0 type vcan
ip link set up vcan0
./can-drpc-130 -c vcan0 -s /dev/ttyACM0 -b 250000
```
