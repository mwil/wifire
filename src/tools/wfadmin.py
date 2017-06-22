#! /usr/bin/env python3

import argparse
import socket
import struct

USRP2_IP = "192.168.10.5"
WIFIRE_PORT = 49154

WIFIRE_CMD_NOP = 0
WIFIRE_CMD_SET_OPMODE = 1
WIFIRE_CMD_SET_PERIODIC = 2
WIFIRE_CMD_SET_TX_SCALE = 3
WIFIRE_CMD_SET_REACT_DELAY = 4
WIFIRE_CMD_SET_SYM_BUF = 5

WIFIRE_OPMODE_IDLE = 0
WIFIRE_OPMODE_REACT = 1
WIFIRE_OPMODE_TX_ZIGBEE = 2

WIFIRE_REPLY_FAILURE = 0
WIFIRE_REPLY_SUCCESS = 1


#
# Input pre-processing
#
def opmode(string):
    opmodes = {"idle": WIFIRE_OPMODE_IDLE,
               "react": WIFIRE_OPMODE_REACT,
               "tx_zigbee": WIFIRE_OPMODE_TX_ZIGBEE}

    return opmodes[string.lower()]


def int_or_hex(string):
    try:
        return (int(string) if string[:2] != "0x" else int(string, 16))
    except ValueError:
        raise argparse.ArgumentTypeError(
                "{} is not an integer!".format(string))


def byte_literal(string):
    res = [0]*64

    if len(string) % 2:
        raise argparse.ArgumentTypeError(
                "{} has an uneven number of nibbles!".format(string))
    else:
        i = 0

        while string:
            res[i] = int(string[:2], 16)
            string = string[2:]
            i += 1

        return [i] + res


class WiFireAction(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.connect((namespace.USRP2_IP, WIFIRE_PORT))

            if isinstance(values, int):
                s.send(struct.pack("!II", self.cmd, values))
            elif isinstance(values, list):
                l = values[0]
                values = values[1:]
                # CMD, 1 byte length, 64 bytes symbols, first sym, last sym
                req = struct.pack("!I B 64B", self.cmd, l, *values)
                req += struct.pack("BB", 0, l*2 - 1)

                values = "0x"+"".join(["{:02x}".format(i) for i in values[:l]])

                s.send(req)

            print(self.msgs["req"].format(namespace.USRP2_IP, values))

            s.settimeout(1.0)
            try:
                cmd, flags = struct.unpack("!II", s.recv(2**8))

                if cmd == self.cmd and flags & WIFIRE_REPLY_SUCCESS:
                    print(self.msgs["success"].format(
                        namespace.USRP2_IP, values))
                else:
                    print("<-- WiFire (@{}): Unexpected reply cmd:"
                          "{} flags: '{}'!".format(
                              namespace.USRP2_IP, cmd, flags))

            except socket.timeout:
                print("Error: Timeout, no control response from the USRP2!")


class OpmodeAction(WiFireAction):
    def __init__(self, **kwds):
        super(OpmodeAction, self).__init__(**kwds)

        self.cmd = WIFIRE_CMD_SET_OPMODE
        self.msgs = {"req": "--> WiFire (@{}): Switching to opmode '{}'.",
                     "success": "<-- WiFire (@{}): Switch to '{}' successful!"}


class PeriodicAction(WiFireAction):
    def __init__(self, **kwds):
        super(PeriodicAction, self).__init__(**kwds)

        self.cmd = WIFIRE_CMD_SET_PERIODIC
        self.msgs = {"req": "--> WiFire (@{}): Changing periodic to {}.",
                     "success": "<-- WiFire (@{}):"
                                "Setting Periodic to {} successful!"}


class TXScaleAction(WiFireAction):
    def __init__(self, **kwds):
        super(TXScaleAction, self).__init__(**kwds)

        self.cmd = WIFIRE_CMD_SET_TX_SCALE
        self.msgs = {
            "req": "--> WiFire (@{}): Changing TX scale to {:=#010x}.",
            "success": "<-- WiFire (@{}): "
                       "Setting TX Scale to {:=#010x} successful!"}


class RDelayAction(WiFireAction):
    def __init__(self, **kwds):
        super(RDelayAction, self).__init__(**kwds)

        self.cmd = WIFIRE_CMD_SET_REACT_DELAY
        self.msgs = {
            "req": "--> WiFire (@{}): Changing reaction delay to {} ticks.",
            "success": "<-- WiFire (@{}): "
                       "Setting reaction delay to {} ticks successful!"}


class SymbolsAction(WiFireAction):
    def __init__(self, **kwds):
        super(SymbolsAction, self).__init__(**kwds)

        self.cmd = WIFIRE_CMD_SET_SYM_BUF
        self.msgs = {
            "req": "--> WiFire (@{}): Changing TX symbols to {}.",
            "success": "<-- WiFire (@{}): "
                       "Setting TX symbols to {} successful!"}


if __name__ == "__main__":
    p = argparse.ArgumentParser(
            description="WiFire administration tool (python style).")

    p.add_argument("--ip", dest="USRP2_IP", action="store", default=USRP2_IP,
                   help="Choose the IP address of the USRP2 instance.")
    p.add_argument("--opmode", action=OpmodeAction, type=opmode,
                   help="WiFire Opmode to send to the USRP2 "
                        "[idle, react, tx_zigbee].")
    p.add_argument("--periodic", action=PeriodicAction,
                   default=50000000, type=int,
                   help="Periodic interrupt that triggers actions; "
                   "specify the number of clock cycles to wait [50MHz clock].")
    p.add_argument("--tx_scale", action=TXScaleAction,
                   default=0x04000400, type=int_or_hex,
                   help="Scaling factor that the TX waveform is scaled with.")
    p.add_argument("--delay", action=RDelayAction, default=4800, type=int,
                   help="Ticks to wait after an SFD/header was detected.")
    p.add_argument("--syms", action=SymbolsAction,
                   default="", type=byte_literal,
                   help="Specify the packet for the TX symbol buffer.")

    args = p.parse_args()
