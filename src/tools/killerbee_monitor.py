#!/usr/bin/env python2
from __future__ import print_function

import struct
import sys

import killerbee


class RZVictim(object):
    def __init__(self, channel=26):
        self.kb = killerbee.KillerBee()
        self.kb.set_channel(channel)
        self.kb.sniffer_on()

        print("RZVictim: listening on '%s', link-type DLT_IEEE802_15_4, "
              "capture size 127 bytes" % self.kb.get_dev_info()[0])
        # print("\nFCF  #  PAN  dest src  6LoW payload           CRC")

    def __del__(self):
        self.kb.sniffer_off()
        self.kb.close()

    def pnext(self):
        packet = None

        while not packet:
            packet = self.kb.pnext()

        return packet


parser = killerbee.Dot154PacketParser()
filename = sys.argv[1]
logfile = open(filename, "w")


def log(x):
    global logfile

    print(x)
    logfile.write(x + "\n")


r = RZVictim()
try:
    while True:
        p = r.pnext()
        # print(p)
        crc_state = p["validcrc"]

        if not crc_state:
            log("Frame with invalid CRC received.")
            continue

        try:
            packet = parser.pktchop(p[0])
        except:
            log("Undecodeable packet: %s" % (p))
            continue

        src, dst, seq = 0, 0, 0

        if packet[1] and packet[3] and packet[5]:  # has src, dst and seqno
            # parsed data in native endianess
            try:
                data = struct.unpack("=BHH", packet[1]+packet[3]+packet[5])
            except struct.error:
                log("Undecodeable packet: %s" % (packet))
                continue

            src = data[2]
            dst = data[1]
            seq = data[0]

            # FCF | Seq# | DPAN | DA | SPAN | SA | [Beacon Data] | PHY Payload
            log("%d(%s) -> %d(%s) (#%d): Len: %d, Payload-Len: "
                "%d, FCF:%s, Payload:%s, crc:%s" %
                (src, repr(packet[4]), dst, repr(packet[2]), seq, len(p[0]),
                 len(packet[7]), repr(packet[0]), repr(packet[7]), crc_state))

        elif packet[0][0] == "\x02":  # ack frame
            seq = ord(packet[1])

            log("ACK: #%d, crc:%s" % (seq, crc_state))
        else:   # uninteresting packet
            log(packet)

        # payload in network endianess!
        if packet[7][:2] == "\x3F\x70":    # ctp routing frame
            # byte 4 seems to be sequence number, byte 3 seems to be constant 0
            try:
                data = struct.unpack("!BHH", packet[7][4:9])
            except struct.error:
                print("Error decoding CTP routing frame")
                continue

            option = data[0]
            parent = data[1]
            etx = data[2]
            # remaining two bytes seem to be crc

            log("  CTP Routing: option:%d, parent:%d, etx:%d" %
                (option, parent, etx))

        elif packet[7][:2] == "\x3F\x71":   # ctp data frame
            try:
                data = struct.unpack("!BBHHBB", packet[7][2:10])
            except struct.error:
                print("Error decoding CTP data frame")
                continue

            option = data[0]
            thl = data[1]
            etx = data[2]
            origin = data[3]
            seqno = data[4]
            collect_id = data[5]
            payload = packet[7][10:-2]

            log("  CTP Data: option:%d, thl:%d, etx:%d, origin:%d, "
                "seqno:%d, collect_id:%d, data:%s" %
                (option, thl, etx, origin, seqno, collect_id, repr(payload)))

        elif packet[7][:2] == "\x3F\x73":        # lqi routing frame
            try:
                data = struct.unpack("!HHHHHH", packet[7][2:14])
            except struct.error:
                print("Error decoding LQI routing frame")
                continue

            originaddr = data[0]
            seqno = data[1]
            originseqno = data[2]
            parent = data[3]
            cost = data[4]
            hopcount = data[5]

            log("  LQI routing: origin:%d, seqno:%d, originseqno:%d, "
                "parent:%d, cost:%d, hopcount:%d" %
                (originaddr, seqno, originseqno, parent, cost, hopcount))

        elif packet[7][:2] == "\x3F\x74":        # lqi data frame
            try:
                data = struct.unpack("!HHHHBH", packet[7][2:13])
            except struct.error:
                print("Error decoding LQI data frame")
                continue

            originaddr = data[0]
            seqno = data[1]
            originseqno = data[2]
            hopcount = data[3]
            collectionid = data[4]
            payload = data[5]

            log("  LQI data: origin:%d, seqno:%d, originseqno:%d, "
                "hopcount:%d, collect_id:%d, data:%d" %
                (originaddr, seqno, originseqno,
                    hopcount, collectionid, payload))

        elif packet[7][:2] == "\x3F\xAA":        # measurement information
            try:
                data = struct.unpack("!IHHHH", packet[7][2:14])
            except struct.error:
                print("Error decoding measurement information")
                continue

            timediff = data[0]
            framecount = data[1]
            duplicates = data[2]
            packetcount = data[3]
            missedcount = data[4]

            log("  Measurement packet: framecount:%d, packetcount:%d, "
                "missedcount:%d, timediff:%d, duplicates:%d" %
                (framecount, packetcount, missedcount, timediff, duplicates))

except KeyboardInterrupt:
    logfile.close()
