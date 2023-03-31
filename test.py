
import os
dsk = open(r"\\.\physicaldrive1",'rb+')

def writedisp(arr):
    dsk.seek(0)
    dsk.write(bytearray(arr))
def writesen(arr):
    dsk.seek(2*160*128)
    dsk.write(bytearray(arr))
arr = [255]*(128*160*2)
writedisp(arr)
#writedisp(arr)
arr = [255]*(512*2)
#writedisp(arr)

#for i in range(100):

