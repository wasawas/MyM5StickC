import sensor
import image
import lcd
import time
import utime
from machine import UART
from Maix import GPIO
from fpioa_manager import *

fm.register(34,fm.fpioa.UART1_TX)
fm.register(35,fm.fpioa.UART1_RX)
uart_out = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.run(1)

while False:
    uart_out.write('TEST\n')
    utime.sleep_ms(100)

# Default Ball
#target_lab_threshold = (45,   70,  -60,   -30,   0,   40)



while True:
    sensor.set_hmirror(False)
    img=sensor.snapshot()

    data = uart_out.read(4096)
    if data:

        if data[0] == 0xAF:
            # JoyC remote detect
            target_lab_threshold = (51, 88, 126, 15, 89, -12)
            print('Remote:' + str(data[0]))
        elif data[0] == 0xBF:
            print('Paper:' + str(data[0]))
            # White paper
            target_lab_threshold = (100, 90, 127, -128, 127, -128)

    blobs = img.find_blobs([target_lab_threshold],
    x_stride = 2, y_stride = 2, pixels_threshold = 100, merge = True, margin = 5)
    if blobs:
        max_y = 0
        #max_area = 0
        target = blobs[0]
        for b in blobs:
            #if b.area() > max_area:
            #    max_area = b.area()
            #    target = b

            if b[1]+b[3] > max_y:
                max_y = b[1]+b[3]
                target = b

        sx = 0
        sy = 0
        wd = 0
        ht = 0
        xx = 0
        cx = 0
        cy = 0
        area = 0



        if data:
            if data[0] == 0xAF:
                print('Remote:' + str(data[0]))
            elif data[0] == 0xBF:
                print('Paper:' + str(data[0]))

            area = target.area()
            #dx = 160-target[5]
            #hexlist = [
            #    (dx >> 8) & 0xFF,
            #    dx & 0xFF,
            #    (area >> 16) & 0xFF,
            #    (area >> 8) & 0xFF,
            #    area & 0xFF
            #]

            sx = target[0]
            sy = target[1]
            wd = target[2]
            ht = target[3]
            xx = target[4]
            cx = target[5]
            cy = target[6]
            hexlist = [
                (sx >> 8) & 0xFF,sx & 0xFF,(sy >> 8) & 0xFF,sy & 0xFF,
                (wd >> 8) & 0xFF,wd & 0xFF,(ht >> 8) & 0xFF,ht & 0xFF,
                (cx >> 8) & 0xFF,cx & 0xFF,(cy >> 8) & 0xFF,cy & 0xFF,
                (area >> 16) & 0xFF,(area >> 8) & 0xFF,area & 0xFF
            ]
            uart_out.write(bytes(hexlist))
        else:
            pass
        #print(target.area())
        #target=[0,0,320,240,100,160,120]

        print('x:' + str(sx) + ' y:' + str(sy) +
        ' w:' + str(wd) + ' h:' + str(ht) + ' 4:' + str(target[4]) +
        ' cx:' + str(cx) + ' cy:' + str(cy) + ' area:' +str(area))

        tmp=img.draw_rectangle(target[0:4],color=(255,0,0))
        tmp=img.draw_cross(target[5], target[6])
        c=img.get_pixel(target[5], target[6])


    else:
        #if uart_out.read(4096):
        if data:
            hexlist = [0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00]
            uart_out.write(bytes(hexlist))
        else:
            pass
