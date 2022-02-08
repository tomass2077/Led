from time import sleep
import cv2
import numpy as np
import os
def Convertion(Nam):
    #Nam = 'RGBW.mp4'
    cap = cv2.VideoCapture(Nam)
    frameCount = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    frameWidth = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    frameHeight = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    print(frameCount)
    print(frameWidth)
    print(frameHeight)

    buf = np.empty((frameCount, frameHeight, frameWidth, 3), np.dtype('uint8'))

    fc = 0
    ret = True

    while (fc < frameCount  and ret):
        ret, buf[fc] = cap.read()
        fc += 1

    cap.release()
    Nam = Nam.replace(".mp4",".dat")
    file= open(str(frameCount)+"_" + Nam,"w+")
    for f in range(frameCount):
        sleep(0.04)
        print("\n");
        i = 0;
        
        for x in range(9):
             #print("");
             for y in range(3):
                 file.write(chr(int(  buf[f][5+y*10][5+x*10][2]/2  )))
                 file.write(chr(int(  buf[f][5+y*10][5+x*10][1]/2  )))
                 file.write(chr(int(  buf[f][5+y*10][5+x*10][0]/2  )))
                 file.write(chr(2))
                 file.write(chr(3))
                 #print(str(i)+" ", end = '')
                # i=i+1
        
        for y in range(3):
            print("");
            for x in range(9):
                 c = ""
                 v = buf[f][5+y*10][5+x*10][2]
                 if(v < 255/5*1):
                     c="."
                 elif(v < 255/5*2):
                     c="-"
                 elif(v < 255/5*3):
                     c="+"
                 elif(v < 255/5*4):
                         c="#"
                 else:
                     c="@"
                 print(c+" ", end = '')
                 i=i+1
                 #print(i, end = '')
                 

    print("\n");
    file.close()

path = os.path.dirname(os.path.realpath(__file__))
files = os.listdir(path)

for f in files:
     split_tup = os.path.splitext(f)
     if(split_tup[1] == ".mp4"):
         print(f)
         sleep(1)
         Convertion(f);
