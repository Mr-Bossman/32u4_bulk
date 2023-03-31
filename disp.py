#!/bin/python3
from time import sleep
import sys
import cv2
#dsk = open(r"\\.\physicaldrive1",'rb+')
class hardwareD:
	def __init__(self,drive):
		self.dsk = open(drive,'rb+') # open drive that is memory map of the display and buzzer
		self.drive = drive # save path

	def writedisp(self,arr):
		if len(arr)%512: # if not multiple of 512(sector size) round up to nearest
			arr += [0]*(512 - len(arr)%512)
		self.dsk.seek(0) # seek to begining of display
		self.dsk.write(bytearray(arr)) # write display
		self.dsk.close() # sync io reads
		self.dsk = open(self.drive,'rb+') # sync io reads and open

if len(sys.argv) != 3:
	print("you're bad at what you do [dev image]")
	exit()

disp = hardwareD(sys.argv[1]) # open drive for me its /dev/sde on windows its `r"\\.\physicaldrive"+ number` 
# `wmic diskdrive list` will list drive number in windows. if the drive number is a real drive it will be erased be VERY carefull
cap = cv2.VideoCapture(sys.argv[2]) # open image for testing
if (cap.isOpened()== False): # it didnt open
	print("Error opening video stream or file")

while(cap.isOpened()):
	ret, img = cap.read() # read image data
	if ret == True: # if end of video

		img = cv2.rotate(img, cv2.ROTATE_90_CLOCKWISE) # rotate image lcd is rotated idk why
		img = cv2.resize(img,(128,160),interpolation = cv2.INTER_LANCZOS4) # scale image to lcd size using LANCZOS4
		img = cv2.cvtColor(img,cv2.COLOR_BGR2BGR565) # convert to bgr565 color space the color space the lcd uses
		arr=[]
		for i in range(160):
			for j in range(128):
				arr+= [img[i,j][1],img[i,j][0]] # because idk python well we expand the 3d array to 1d and swap the bytes.
		disp.writedisp(arr) # write the binary data to the display
	else:
		break

cap.release()
cv2.destroyAllWindows()

print(disp.readtemp()) # read temp
sleep(1) # wait for recalc
