#stolen from https://hackthedeveloper.com/motion-detection-opencv-python/

import cv2
import pygame
import serial 
import time

#arduino = serial.Serial(port='COM10', baudrate=9600, timeout=.5) # JH commented out for testing
arduino = serial.Serial(port='/dev/ttyACM0', baudrate=9600, timeout=.5) # JH commented out for testing

print(arduino.name)         # check which port was really used
# set these values as you need
AreasHorizontal = 4
AreasVertical = 4
#thresholds = [100] * AreasHorizontal * AreasVertical
thresholds = [10,11,12,13,20,21,22,23,30,31,32,33,40,41,42,43]
deviceMap = ["G20","E3","H45","A60","K60","C300","J60","L60","H1","L60","K60","M200","B15","D500","I50","E3"]

realtimeCount = [0] * AreasHorizontal * AreasVertical

def rectanglesOverlap(R1,R2):
    return not((R1[0]>=R2[2]) or (R1[2]<=R2[0]) or (R1[3]<=R2[1]) or (R1[1]>=R2[3]))

pygame.mixer.init()
 
cap = cv2.VideoCapture(2) # (0) for build in camera, (2) for USB camera

mog = cv2.createBackgroundSubtractorMOG2()
# typing = pygame.mixer.Sound('C:/Users/atomi/Dropbox/Python Projects/typewriter.wav')  # JH commented out for testing
# jumping = pygame.mixer.Sound('C:/Users/atomi/Dropbox/Python Projects/game-jump.wav')  # JH commented out for testing
# jumping.play()

while True:
    ret, frame = cap.read()
    height, width = frame.shape[:2]
    sectionX = int(width/AreasHorizontal)
    sectionY = int(height/AreasVertical)

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    
    fgmask = mog.apply(gray)
    
    kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
    fgmask = cv2.erode(fgmask, kernel, iterations=1)
    fgmask = cv2.dilate(fgmask, kernel, iterations=1)

    contours, hierarchy = cv2.findContours(fgmask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    for i in range(0,AreasHorizontal):
        for j in range(0, AreasVertical):
            R1 = [sectionX * i, sectionY * j, sectionX * (i + 1), sectionY * (j + 1)]
            cv2.rectangle(frame, (R1[0],R1[1]), (R1[2],R1[3]), (0,255,0),2)
    
    for contour in contours:
        # Ignore small contours
        if cv2.contourArea(contour) < 1000:
            continue

        # Draw bounding box around contour
        x, y, w, h = cv2.boundingRect(contour)
        cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
        '''
        if (x > 500) and (y > 200):
            print("jumping.play()")
    
        if (x < 200) and (y < 100):
            print("typing.play()")
        '''
        for i in range(0,AreasHorizontal):
            for j in range(0, AreasVertical):
                R1 = [sectionX * i, sectionY * j, sectionX * (i + 1), sectionY * (j + 1)]
                R2 = [x,y,x+w,y+w]
                if rectanglesOverlap(R1,R2):
                    k = i * AreasVertical + j
                    realtimeCount[k] += 1
                    if realtimeCount[k] == thresholds[k]:
                        cv2.rectangle(frame, (R1[0],R1[1]), (R1[2],R1[3]), (0,0,255),10)
                        print(deviceMap[k])  
                        arduino.write(deviceMap[k].encode('utf-8')) 
                        arduino.write('\r\n'.encode('utf-8')) 
                        #print(chr(deviceMap(k)))   #realtimeCount[k],
                        realtimeCount[k] = 0
    
    cv2.imshow('Motion Detection', frame)
    if cv2.waitKey(1) == ord('q'):
        break
        
cap.release()
cv2.destroyAllWindows()
