#!/usr/bin/env python3
from time import time
from rsa import sign
import serial
from serial.threaded import LineReader, ReaderThread

import sys
import json
import struct

from time import sleep


class PrintLines(LineReader):
    TERMINATOR = b'\n'
    def connection_made(self, transport):
        super(PrintLines, self).connection_made(transport)

    def handle_line(self, data):
        print("$ rx: " + data, flush=True)
        if data == "PER":
            exit(0)

    def connection_lost(self, exc):
        if exc:
            sys.stderr.write(str(exc))
        sys.stdout.write('port closed\n')
    


class MyReaderThread(ReaderThread):
    def __init__(self, serial_instance, protocol_factory):
        super(MyReaderThread, self).__init__(serial_instance, protocol_factory)


if __name__ == '__main__':
    cfgFileName = sys.argv[1]
    if cfgFileName.endswith('.json') != True:
        print("please select config json file")
        exit(0)
    with open(cfgFileName, 'r') as f:
        dic = json.loads(f.read())
        lex = dic['host']
    
    lex = 'socket://' + lex + ':3334'
    com = serial.serial_for_url(lex)
    # while True:
    #     print(com.read().decode())



    ser = MyReaderThread(com, PrintLines)
    ser.start()
    h = 0
    while True:
        h += 2
        try:
            k = input()
            # k += '\n'
            
            # k = "B".encode() + \
            #     int.to_bytes(1, 1, 'little', signed=False) + \
            #     int.to_bytes(0, 1, 'little', signed=True) + \
            #     int.to_bytes(0, 1, 'little', signed=True) + \
            #     struct.pack('<f', 90.0) + struct.pack('<f', 0.0) + \
            #     int.to_bytes(0, 2, 'little', signed=False) + \
            #     int.to_bytes(500, 2, 'little', signed=False) + \
            #     int.to_bytes(0, 1, 'little', signed=False) + \
            #     int.to_bytes(1, 1, 'little', signed=False) + \
            #     "\n".encode()
            # print(".", end='', flush=True)
            com.write(k)
            # sleep(0.060)
            
        except KeyboardInterrupt as e:
            print("EXIT KEY TRIGGER")
            ser.stop()
            com.close()
            break