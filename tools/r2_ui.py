#!/usr/bin/env python3
from time import time
from rsa import sign
import serial
from serial.threaded import LineReader, ReaderThread

import sys
import json
import struct

import pygame
from pygame.locals import *

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

    pygame.init()
    screen = pygame.display.set_mode((320,240))

    mode = 0
    mvAngle = 0
    mvSpeed = 0

    jidianqi = 1

    x1 = 0
    x2 = 0
    pl = 0
    plm = 1
    zhuaE = 95
    zhuaR = 0
    

    while True:
        h += 2
        try:
            for event in pygame.event.get():
                if event.type == QUIT:
                    exit(0)
                
                if event.type == KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        exit(0)
                    elif event.key == pygame.K_LEFT:
                        mvAngle -= 1
                    elif event.key == pygame.K_RIGHT:
                        mvAngle += 1
                    elif event.key == pygame.K_UP:
                        if mvSpeed < 0:
                            mvSpeed = 0
                        else:
                            mvSpeed += 1
                    elif event.key == pygame.K_DOWN:
                        if mvSpeed > 0:
                            mvSpeed = 0
                        else:
                            mvSpeed -= 1
                    elif event.key == pygame.K_SPACE:
                        mvSpeed = 0
                    elif event.key == pygame.K_1:
                        mode = 0
                        mvAngle = 0
                        mvSpeed = 0
                    elif event.key == pygame.K_2:
                        mode = 1
                        mvAngle = 0
                        mvSpeed = 0
                    elif event.key == pygame.K_3:
                        mode = 2
                        mvAngle = 0
                        mvSpeed = 0
                    elif event.key == pygame.K_TAB:
                        if jidianqi == 0:
                            jidianqi = 1
                        else:
                            jidianqi = 0
                    elif event.key == pygame.K_q:
                        x1 -= 100
                        if x1 < 0:
                            x1 = 0
                    elif event.key == pygame.K_w:
                        x1 += 100
                    elif event.key == pygame.K_a:
                        x2 -= 100
                        if x2 < 0:
                            x2 = 0
                    elif event.key == pygame.K_s:
                        x2 += 100
                    elif event.key == pygame.K_z:
                        pl -= 50
                        if pl < 0:
                            pl = 0
                    elif event.key == pygame.K_x:
                        plm = 0
                        pl = 50
                    elif event.key == pygame.K_e:
                        plm = 2
                        pl = 0
                    elif event.key == pygame.K_r:
                        zhuaE -= 5
                        if zhuaE < 5:
                            zhuaE = 5
                    elif event.key == pygame.K_d:
                        zhuaR -= 5
                        if zhuaR < 0:
                            zhuaR = 0
                    elif event.key == pygame.K_f:
                        zhuaR += 5
                        if zhuaR > 90:
                            zhuaR = 90
                    


                    k = "B".encode() + \
                        int.to_bytes(mode, 1, 'little', signed=False) + \
                        int.to_bytes(zhuaE, 1, 'little', signed=True) + \
                        int.to_bytes(zhuaR, 1, 'little', signed=True) + \
                        struct.pack('<f', mvAngle) + struct.pack('<f', mvSpeed) + \
                        int.to_bytes(x1, 2, 'little', signed=False) + \
                        int.to_bytes(x2, 2, 'little', signed=False) + \
                        int.to_bytes(pl, 1, 'little', signed=False) + \
                        int.to_bytes(plm + jidianqi * (1 << 4), 1, 'little', signed=False) + \
                        "\n".encode()
                    print(".", end='', flush=True)
                    com.write(k)
            
        except KeyboardInterrupt as e:
            print("EXIT KEY TRIGGER")
            ser.stop()
            com.close()
            break