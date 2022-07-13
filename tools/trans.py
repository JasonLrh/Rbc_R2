#!/usr/bin/env python3
from time import time
import serial
from serial.threaded import LineReader, ReaderThread

import sys
import json

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
        h += 0.5
        try:
            # k = input()
            # # print("[input]", k)
            # k += '\n'
            k = """J{"m":[0,0],"f":{"h":%d,"r":45,"e":180},"s":{"h":500,"x":0,"isSuck":false}}"""%(h)
            com.write(k.encode())
            sleep(0.012)
        except KeyboardInterrupt as e:
            print("EXIT KEY TRIGGER")
            ser.stop()
            com.close()
            break