#from asyncio import windows_events
#from ctypes import alignment
#from dataclasses import replace
#from logging import PlaceHolder
from msilib.schema import TextStyle
#from re import X
#from socket import timeout
#from telnetlib import IP
import tkinter as tk
import tkinter.ttk as ttk
from tkinter import CENTER, filedialog as fd
from tkinter.colorchooser import askcolor
from numpy import spacing
from ttkthemes  import ThemedTk
#from ttkthemes  import ThemedStyle
from time import sleep
import cv2
import numpy as np
#import os
#import math
from functools import partial
#from ttkwidgets import Calendar
import json
from tkintermapview import TkinterMapView
#import sys
import serial
import serial.tools.list_ports

window = ThemedTk(theme="equilux")
window.geometry("300x350")
window.title("Tools")
style = ttk.Style(window)
window.resizable(False, False)
style.theme_use('equilux')
window.configure(background='#222222')
style.configure('TButton', foreground='#DDDDDD')
style.configure('TLabel', foreground='#DDDDDD')

style.configure('TButton', background = '#222222')
style.configure('TLabel', background='#222222')
style.configure('TEntry', background='#222222')
style.configure('TCheckButton', background='#222222')
style.configure('TOptionmenu', background='#222222')
style.configure('TSpinbox', background='#222222')



#app.tk.call("set_theme", "dark")
ProgressLabel = ttk.Label(text="progress:0%",borderwidth=2)
def Convertion(Nam,loc): 
    global ProgressLabel
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
    Nam = Nam.replace(".mp4",".dat").split('/')[-1]
    file= open(loc+str(frameCount)+"_" + Nam,"w+")
    for f in range(frameCount):
        sleep(0.01)
        i = 0
        
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
        ProgressLabel.config(text="progress:"+str(round(f/(frameCount-1)*100))+"%")
        ProgressLabel.update()
    file.close()

#Colors = [ ["#000000"]*9 ]*3
#ColorButtons = [ [tk.Button(window, text = '')]*9 ]*3

Colors = [["#000000" for i in range(9)] for j in range(3)]
ColorButtons = [[tk.Button(window, text = '') for i in range(9)] for j in range(3)]
def hex_to_rgb(value):
    value = value.lstrip('#')
    lv = len(value)
    return tuple(int(value[i:i + lv // 3], 16) for i in range(0, lv, lv // 3))

filenames = []
folder = ""
FileLabel = ttk.Label(text=str('\n'.join(filenames)),borderwidth=2)
DestinationLabel = ttk.Label(text=folder,borderwidth=2)
def convert():
    global ProgressLabel
    for files in filenames:
        ProgressLabel = ttk.Label(text="progress:0%",borderwidth=2)
        ProgressLabel.pack()
        Convertion(files,folder)
def select_files():
    global filenames
    global FileLabel
    if(len(filenames)==0):
        filetypes = (
            ('files', '*.mp4'),
        )

        filenames = fd.askopenfilenames(
            title='Open files',
            initialdir='/',
            filetypes=filetypes)
        FileLabel = ttk.Label(text=str('\n'.join(filenames)),borderwidth=2)
        FileLabel.pack(pady=3)
        ttk.Button(window ,text='Select destination',command=select_Destination).pack()
    else:
        filetypes = (
            ('files', '*.mp4'),
        )

        filenames = fd.askopenfilenames(
            title='Open files',
            initialdir='/',
            filetypes=filetypes)
        FileLabel.config(text=str('\n'.join(filenames)))
        FileLabel.update()
def select_Destination():
    global folder
    global DestinationLabel
    if(folder == ""):
        folder = fd.askdirectory(initialdir="/", title="Open destination")
        DestinationLabel = ttk.Label(text=folder,borderwidth=2)
        DestinationLabel.pack(pady=3)
        ttk.Button(window ,text='Make files',command=convert).pack()
    else:
        folder = fd.askdirectory(initialdir="/", title="Open destination")
        DestinationLabel.config(text=folder)
        DestinationLabel.update()
def ColPallet(x,y):
    global ColorButtons
    global Colors
    #y=math.floor(p/9)
    #x=p-y*9
    Colors[y][x] = (askcolor(title = "Color Chooser",color=Colors[y][x])[1])
    ColorButtons[y][x].config(bg = Colors[y][x])
    ColorButtons[y][x].update()
def ColorGrid():
    global ColorButtons
    global Colors
    for y in range(3):
        for x in range(9):
             print(str(x)+" "+str(y))
             action = partial(ColPallet, (x+y*9))
             val = x+y*9
             col = Colors[y][x]
             ColorButtons[y][x] = tk.Button(window, text = '',command=lambda x=x,y=y: ColPallet(x,y),bg = col)
             if(x>5):
                 ColorButtons[y][x].place(x=50+x*25, y=10 + y*25,width=24,height=24)
             else:
                 ColorButtons[y][x].place(x=40+x*25, y=10 + y*25,width=24,height=24)
            
def AnimMaker():
    for widget in window.winfo_children():
         widget.destroy()
    ttk.Button(window ,text='Open files',command=select_files).pack()
Name = tk.StringVar()
def ImgConvertion(Nam,loc): 
    global ProgressLabel
    global Colors
    buf = Colors
    fc = 0
    ret = True
    file= open(loc+"/0"+"_" + Nam+".dat","w+")
    for x in range(9):
         #print("");
         for y in range(3):
             sleep(0.01)
             file.write(chr(int(  hex_to_rgb(buf[y][x])[2]/2  )))
             file.write(chr(int(  hex_to_rgb(buf[y][x])[1]/2  )))
             file.write(chr(int(  hex_to_rgb(buf[y][x])[0]/2  )))
             file.write(chr(2))
             file.write(chr(3))
             ProgressLabel.config(text="progress:"+str(round((x*3+y)/26*100))+"%")
             ProgressLabel.update()
                 #print(str(i)+" ", end = '')
            # i=i+1      
    file.close()
def convertImg():
    global ProgressLabel
    ProgressLabel = ttk.Label(text="progress:0%",borderwidth=2)
    ProgressLabel.place(y=250,x=150,anchor="center")
    ImgConvertion(Name.get(),folder)
def select_DestinationImg():
    global folder
    global DestinationLabel
    if(folder == ""):
        folder = fd.askdirectory(initialdir="/", title="Open destination")
        DestinationLabel = ttk.Label(text=folder,borderwidth=2)
        DestinationLabel.place(y=190,x=150,anchor="center")
        ttk.Button(window ,text='Make files',command=convertImg).place(y=220,x=150,anchor="center")
    else:
        folder = fd.askdirectory(initialdir="/", title="Open destination")
        DestinationLabel.config(text=folder)
        DestinationLabel.update()
def ImageMaker():
    for widget in window.winfo_children():
         widget.destroy()
    ColorGrid()
    ttk.Label(window,text="Name:",borderwidth=2,).place(y=100,x=150,anchor="center")
    ttk.Entry(window,textvariable=Name).place(y=120,x=150,anchor="center")
    ttk.Button(window ,text='Select destination',command=select_DestinationImg).place(y=160,x=150,anchor="center")
Ssid = tk.StringVar()
Pass = tk.StringVar()
Ip = tk.StringVar()
Id = tk.StringVar()
checkSun=tk.BooleanVar()
checkTime=tk.BooleanVar()
MaxSunAngle = tk.StringVar()
def is_digit(n):
    try:
        int(n)
        return True
    except ValueError:
        return  False
hours = ['00', '01', '02', '03', '04', '05', '06', '07',
         '08', '09', '10', '11', '12', '13', '14', '15',
         '16', '17', '18', '19', '20', '21', '22', '23'
        ]
minutes = ['00', '10', '20', '30', '40', '50',]
hour1 = tk.StringVar()
hour1.set(hours[0])
minute1 = tk.StringVar()
minute1.set(minutes[0])

hour2 = tk.StringVar()
hour2.set(hours[0])
minute2 = tk.StringVar()
minute2.set(minutes[0])
filename=""
data = ""
map_widget = TkinterMapView()
marker = map_widget.set_marker(0, 0)
window2 = ""
def mapUpdate():
    global map_widget
    global marker
    marker.set_position( map_widget.get_position()[0], map_widget.get_position()[1])
    map_widget.update()
    window2.after(10, mapUpdate)
def CloseMap():
    data["Latitude"] = map_widget.get_position()[0]
    data["Longitude"] = map_widget.get_position()[1]
    window2.destroy()
def ConfigMap():
    global map_widget
    global marker
    global window2
    window2 = tk.Toplevel(window)
    window2.geometry("400x400")
    window2.title("Map")
    style2 = ttk.Style(window2)
    window2.resizable(False, False)
    style2.theme_use('equilux')
    window2.configure(background='#222222')
    map_widget = TkinterMapView(window2, width=400, height=400, corner_radius=0)
    map_widget.set_position(data["Latitude"], data["Longitude"])
    map_widget.set_zoom(12)
    #marker = map_widget.set_marker(data["Latitude"], data["Longitude"])
    marker = map_widget.set_marker(map_widget.get_position()[0], map_widget.get_position()[1])
    map_widget.place(relx=0.5, rely=0.5, anchor=tk.CENTER)
    ttk.Button(window2 ,text='Save',command=CloseMap).pack(side="bottom",anchor="sw")
    window2.after(10, mapUpdate)
    window2.mainloop()
def ConfigSave():
    data["ssid"] = Ssid.get()
    data["pass"] = Pass.get()
    data["MaxSunAngle"] = MaxSunAngle.get()
    data["CheckSunAngle"] =checkSun.get()
    data["CheckTime"] =checkTime.get()
    data["StopInMorning"] = hour1.get() + ":"+minute1.get()
    data["StartInEvening"] = hour2.get() + ":"+minute2.get()
    f= open(filename,'w')
    f.write(json.dumps(data,indent=2, sort_keys=True))
    f.close()
    print(data)
def callback(P):
    print(P) 
    if is_digit(P):
        return True           
    elif P == "":
        return True
  
    else:
        return False
def Config():
    for widget in window.winfo_children():
         widget.destroy()
    filetype = (
            ('json', '*.json'),
        )
    global filename
    filename = fd.askopenfilename(
        title='Open config',
        initialdir='/',
        filetype=filetype)
    global data
    f= open(filename, 'r')
    data = json.load(f)
    f.close()
    Ssid.set(data["ssid"])
    Pass.set(data["pass"])
    MaxSunAngle.set(data["MaxSunAngle"])
    checkSun.set(data["CheckSunAngle"])
    checkTime.set(data["CheckTime"])

    hour1.set(str(data["StopInMorning"]).split(":")[0])
    minute1.set(str(data["StopInMorning"]).split(":")[1])
    hour2.set(str(data["StartInEvening"]).split(":")[0])
    minute2.set(str(data["StartInEvening"]).split(":")[1])
    Row = 0
    ttk.Label(window,text="Network name:",borderwidth=2).grid(row=Row, column=0)
    ttk.Entry(window,textvariable=Ssid).grid(row=Row, column=1)
    Row+=1
    ttk.Label(window,text="Network password:",borderwidth=2).grid(row=Row, column=0)
    ttk.Entry(window,textvariable=Pass).grid(row=Row, column=1)
    Row+=1
    ttk.Label(window,text="Off Sun angle:",borderwidth=2).grid(row=Row, column=0)
    reg = (window.register(callback))
    ttk.Entry(window,textvariable=MaxSunAngle,validate="all", validatecommand=(reg, '%P')).grid(row=Row, column=1)
    Row+=1
    ttk.Label(window,text="Check sun angle:",borderwidth=2).grid(row=Row, column=0)
    ttk.Checkbutton(window,variable=checkSun,onvalue=True,offvalue=False).grid(row=Row, column=1)
    Row+=1
    ttk.Label(window,text="End in morning at:",borderwidth=2).grid(row=Row, column=0)
    ttk.OptionMenu(window, hour1, data["StopInMorning"].split(":")[0], *hours).grid(row=Row, column=1)
    ttk.OptionMenu(window, minute1, data["StopInMorning"].split(":")[1], *minutes).grid(row=Row,column=1,sticky="E")
    Row+=1
    ttk.Label(window,text="Start in evening at:",borderwidth=2).grid(row=Row, column=0)
    ttk.OptionMenu(window, hour2, data["StartInEvening"].split(":")[0], *hours).grid(row=Row, column=1)
    ttk.OptionMenu(window, minute2, data["StartInEvening"].split(":")[1], *minutes).grid(row=Row,column=1,sticky="E")
    Row+=1
    ttk.Label(window,text="Check Time:",borderwidth=2).grid(row=Row, column=0)
    ttk.Checkbutton(window,variable=checkTime,onvalue=True,offvalue=False).grid(row=Row, column=1)
    Row+=1
    ttk.Button(window ,text='Map',command=ConfigMap,padding=2).grid(row=Row, column=1)
    Row+=1
    Row+=1
    ttk.Button(window ,text='Save',command=ConfigSave,padding=2).grid(row=Row, column=1)
    #ttk.Button(window ,text='Select destination',command=select_DestinationImg).place(y=160,x=150,anchor="center")
    
AvailableCom = []
def Com():
    for widget in window.winfo_children():
         widget.destroy()
    ttk.Label(text="Plug in addapter",borderwidth=2).pack()
    ports = serial.tools.list_ports.comports()
    global AvailableCom
    AvailableCom = []
    for p in ports:
        AvailableCom.append(p.name)
    window.after(10, SearchCom)
UploadStage=0
ser = ""
Row = 6
UpoadPort = ""
def UploadProcedure():
    global UploadPort
    global UploadStage
    global ser
    global Row
    if(UploadStage==-1):
        UploadStage =0
    if(UploadStage==0):
        UploadStage+=1
        ttk.Label(window,text="Connect controller",borderwidth=2).grid(row=Row, column=1)
        
        ser.flushInput()
        ser.flushOutput()
    elif(UploadStage==1):
        data = ser.readline()
        if(data == b'Started\r\n'):
            UploadStage+=1
    elif(UploadStage==2):
        UploadStage+=1
        ttk.Label(window,text="Got controller",borderwidth=2).grid(row=Row+1, column=1)
        data = ser.readline()
    elif(UploadStage==3):
        ttk.Label(window,text="Writing ssid",borderwidth=2).grid(row=Row+2, column=1)
        ser.flushInput()
        ser.write(("ssid"+Ssid.get()).encode('utf-8'))
        data = ser.readline()
        if(data.decode('utf-8')=="ssid set to:"+Ssid.get()):
            ttk.Label(window,text=data,borderwidth=2).grid(row=Row+3, column=1)
            UploadStage+=1
            data = ser.readline()
    elif(UploadStage==4):
        ttk.Label(window,text="Writing pass",borderwidth=2).grid(row=Row+4, column=1)
        ser.write(("pass"+Pass.get()).encode('utf-8'))
        data = ser.readline()
        if(data.decode('utf-8')=="pass set to:"+Pass.get()):
            ttk.Label(window,text=data,borderwidth=2).grid(row=Row+5, column=1)
            UploadStage+=1
    elif(UploadStage==5):
        ttk.Label(window,text="Writing IP",borderwidth=2).grid(row=Row+6, column=1)
        ser.write(("ip"+Ip.get()).encode('utf-8'))
        data = ser.readline()
        if(data.decode('utf-8')=="Ip set to:"+Ip.get()):
            ttk.Label(window,text=data,borderwidth=2).grid(row=Row+7, column=1)
            UploadStage+=1
    elif(UploadStage==6):
        ttk.Label(window,text="Writing ID",borderwidth=2).grid(row=Row+8, column=1)
        ser.write(("id"+Id.get()).encode('utf-8'))
        data = ser.readline()
        if(data.decode('utf-8')=="ID set to:"+Id.get()):
            ttk.Label(window,text=data,borderwidth=2).grid(row=Row+9, column=1)
            UploadStage=-1
            window.after(1000,StipConfig)
    if(UploadStage!=-1):
        window.after(1, UploadProcedure)
def StipConfig():
    for widget in window.winfo_children():
         widget.destroy()
    com=UploadPort
    Row = 0
    ttk.Label(text="Found at:"+com,borderwidth=2).grid(row=Row, column=0)
    Row+=1
    ttk.Label(window,text="Network name:",borderwidth=2).grid(row=Row, column=0)
    ttk.Entry(window,textvariable=Ssid).grid(row=Row, column=1)
    Row+=1
    ttk.Label(window,text="Network password:",borderwidth=2).grid(row=Row, column=0)
    ttk.Entry(window,textvariable=Pass).grid(row=Row, column=1)
    Row+=1
    ttk.Label(window,text="IP:",borderwidth=2).grid(row=Row, column=0)
    ttk.Entry(window,textvariable=Ip).grid(row=Row, column=1)
    Row+=1
    ttk.Label(window,text="ID:",borderwidth=2).grid(row=Row, column=0)
    ttk.Spinbox(window, from_= 0, to = 26,textvariable=Id,width=5,style="TSpinbox").grid(row=Row, column=1)
    Row+=1
    ttk.Button(window ,text='Initiate upload',command=UploadProcedure,padding=2).grid(row=Row, column=1)
serc = True
def SearchCom():
    ports = serial.tools.list_ports.comports()
    global AvailableCom
    global serc
    global UploadPort
    Coms = []
    for p in ports:
        Coms.append(p.name)
    for c in Coms:
        if c not in AvailableCom:
            serc=False
            
            print(c)
            UploadPort=c
            global ser
            ser=serial.Serial(UploadPort,115200,timeout=0.5)
            StipConfig()
    AvailableCom = Coms
    Id.set("0")
    if serc:
        window.after(500, SearchCom)
    
if __name__ == '__main__':
    ttk.Button(window ,text='Make image',command=ImageMaker,padding=2).pack()
    ttk.Button(window ,text='Convert animation',command=AnimMaker,padding=2).pack()
    ttk.Button(window ,text='Edit config',command=Config,padding=2).pack()
    ttk.Button(window ,text='Configure LED strip controller',command=Com,padding=2).pack()
    window.mainloop()