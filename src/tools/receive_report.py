import socket
from struct import *

port = 49155

REP_TUNE_WIFIRE = 2
REP_MSDU = 7
REP_GENERIC = 8

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("", port))

#try:
while True:
    data, addr = sock.recvfrom(1024)
    report = unpack('!I', data[0:4])
    cmd = report[0]
    if cmd == REP_MSDU:
	tmp = unpack('!III', data[4:])
	msdu_len = tmp[0]
	msdu_pos = tmp[1]
	rx_byte = tmp[2]
	#print 'MSDU: len: %d, pos: %d, payload: %s' % (msdu_len, msdu_pos, repr(data[12:12+msdu_len-8]))
	print 'MSDU: len: %d, pos: %d, rx_byte: %s' % (msdu_len, msdu_pos, repr(chr(rx_byte % 255)))
    elif cmd == REP_GENERIC:
	tmp = unpack('!II', data[4:])
	key = tmp[0]
	value = tmp[1]
	print 'Generic: key: %d, value: %d' % (key, value)
    elif cmd == REP_TUNE_WIFIRE:
	tmp = unpack('!IIIIIIIIIIIIIII', data[4:])
	print 'Header: seqno:%d, srcaddr:%d, srcpan:%d, dstaddr:%d, dstpan:%d, verdict:%d' % (tmp[2], tmp[8], tmp[6], tmp[5], tmp[3], tmp[9])
    else:
	print "cmd: %d, len: %d, data: %s" % (cmd, len(data), repr(data))
#except:
#    sock.close()

