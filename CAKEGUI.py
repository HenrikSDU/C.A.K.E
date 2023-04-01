import tkinter
from tkinter import filedialog
from tkinter import *


import os

def filechoose():

    file_path_string = filedialog.askopenfilename()
    print(file_path_string)
    os.system('"D:\AAStudium\Study\Mechatronics Semester Project 2\GUI\pyGUI\stbi_test.exe"')


def winclose():
    
    window.destroy()

window = tkinter.Tk()
window.title("C.A.K.E")
window.geometry("800x500")
window.resizable(0,0)

window.iconbitmap("SDU.ico")
background_img = tkinter.PhotoImage(file="logo.png")
background_lable = tkinter.Label(window, image = background_img)
background_lable.place(x=0,y=0,relwidth=1,relheight=1)

filebutton = tkinter.Button(window, text="Choose file", command=filechoose,width=10)
filebutton.pack(padx=60,pady=60)
#cancelbutton = tkinter.Button(window, text="Close App", command=winclose,width=10)
#cancelbutton.pack(padx=30,pady=30)



window.mainloop()