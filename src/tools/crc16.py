# Taken from the following open source library:
#    Library : lib_crc
#    Author  : Lammert Bies  1999-2008
#    E-mail  : info@lammertbies.nl


def init_crckermit_tab():
    crc_tabkermit = []

    for i in range(256):
        crc, c = 0, i

        for j in range(8):
            crc = (crc >> 1) ^ 0x8408 if (crc ^ c) & 0x0001 else crc >> 1
            c >>= 1

        crc_tabkermit.append(crc)

    return crc_tabkermit


CRC_TAB_KERMIT = init_crckermit_tab()


def update_crc_kermit(crc, c):
    return (crc >> 8) ^ CRC_TAB_KERMIT[(crc ^ c) & 0xFF]


def calc_crc_kermit(data):
    crc = 0

    for byte in data:
        crc = update_crc_kermit(crc, byte)

    # switch byte order
    return ((crc << 8) & 0xFF00) | (crc >> 8)


if __name__ == "__main__":
    packet = [0x41, 0x88, 0x6D, 0x22, 0x00, 0xFF, 0xFF, 0x00,
              0x00, 0x3F, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x04,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
    packet = b"\x41\x88\x6D\x22\x00\xFF\xFF\x00\x00\x3F\x00"\
             b"\xFF\xFF\x00\x00\x04\x00\x00\x00\x00\x00\x00"

    packet = b"\x00\x00\x00\x00\x00\x00\x00\x00\x01\xed\xda\xeb\xfe"

    crc = calc_crc_kermit(packet)
    print("CRC16: {:#x}".format(crc))

    # assert(crc == 0x7ef9)
