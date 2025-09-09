"""
Implementation of the GazeTracking library.
"""
import pyglet #riaz
import cv2
import pyrebase
from gaze_tracking import GazeTracking

# Load sounds riaz
center_sound = pyglet.media.load("center.wav", streaming=False)
left_sound = pyglet.media.load("left.wav", streaming=False)
right_sound = pyglet.media.load("right.wav", streaming=False)
stop_sound = pyglet.media.load("stop.wav", streaming=False)

gaze = GazeTracking()
webcam = cv2.VideoCapture(1)





# Pyrebase
config = {
   # "apiKey": "yourapikey",
	# "authDomain": "nodemcu-6a1b8.firebaseapp.com",
	# "databaseURL": "https://nodemcu-6a1b8-default-rtdb.firebaseio.com",
	# "projectId": "nodemcu-6a1b8",
	# "storageBucket": "nodemcu-6a1b8.appspot.com",
	# "messagingSenderId": "578188294320",
	# "appId": "1:578188294320:web:8028e229c90a3dfc7b2375",
	# "measurementId": "G-WPSY8SRPDW"
    "apiKey": "yourapike",
    "authDomain": "nodemcu-6a1b8-wheelchair.firebaseapp.com",
    "databaseURL": "https://nodemcu-6a1b8-wheelchair-default-rtdb.asia-southeast1.firebasedatabase.app",
    "projectId": "nodemcu-6a1b8-wheelchair",
    "storageBucket": "nodemcu-6a1b8-wheelchair.firebasestorage.app",
    "messagingSenderId": "922719754463",
    "appId": "1:922719754463:web:d87806d94c1d95aa59716a"
	}
firebase=pyrebase.initialize_app(config)
db=firebase.database()
db.child("Position").set("Null")

while True:
    # We get a new frame from the webcam
    _, frame = webcam.read()

    # We send this frame to GazeTracking to analyze it
    gaze.refresh(frame)

    frame = gaze.annotated_frame()
    text = ""


#riaz add
    # after: frame = gaze.annotated_frame()
    h, w = frame.shape[:2]
    x = 20  # left margin
    gap = 28  # line gap (pixels)

    # bottom three lines
    y3 = h - 20  # bottom line
    y2 = y3 - gap  # one line above
    y1 = y2 - gap  # two lines above
#riaz add

    if gaze.is_blinking():
        text = "Stop the wheelchair!"
        db.update({"Position":"Stop"})



        # stop_sound.play()
    elif gaze.is_right():
        text = "Turn Right!"
        db.update({"Position":"Right"})
        # right_sound.play()
    elif gaze.is_left():
        text = "Turn Left!"
        db.update({"Position":"Left"})
        # left_sound.play() #riaz
    elif gaze.is_center():
        text = "Go Straight!"
        db.update({"Position":"Center"})
        # center_sound.play()  # riaz

        # Draw the text on your frame with desired font style
        # cv2.putText(frame, text, (30, 30),  # (x, y) position
        #             cv2.FONT_HERSHEY_SIMPLEX, 0.5,  # font type and scale
        #             (255, 255, 255), 2, cv2.LINE_AA)  # color, thickness, line type

    # cv2.putText(frame, text, (90, 60), cv2.FONT_HERSHEY_DUPLEX, 1.6, (147, 58, 31), 2)
    cv2.putText(frame, text, (90, 60), cv2.FONT_HERSHEY_DUPLEX, 1.6, (245,66,230), 2)

    left_pupil = gaze.pupil_left_coords()
    right_pupil = gaze.pupil_right_coords()
    #removed to below webcam win
    #cv2.putText(frame, "Left pupil:  " + str(left_pupil), (90, 130), cv2.FONT_HERSHEY_DUPLEX, 0.9, (245,66,230), 2)
    #cv2.putText(frame, "Right pupil: " + str(right_pupil), (90, 165), cv2.FONT_HERSHEY_DUPLEX, 0.9, (245,66,230), 2)


    # riaz add
    def draw_label(img, txt, org, font, scale, color, thick):
        (tw, th), base = cv2.getTextSize(txt, font, scale, thick)
        x, y = org
        cv2.rectangle(img, (x - 6, y - th - 6), (x + tw + 6, y + base + 6), (0, 0, 0), -1)
        cv2.putText(img, txt, org, font, scale, color, thick, cv2.LINE_AA)


    draw_label(frame, text, (x, y1), cv2.FONT_HERSHEY_DUPLEX, 1.0, (245, 66, 230), 2)
    draw_label(frame, f"Left pupil:  {left_pupil}", (x, y2), cv2.FONT_HERSHEY_DUPLEX, 0.9, (245, 66, 230), 2)
    draw_label(frame, f"Right pupil: {right_pupil}", (x, y3), cv2.FONT_HERSHEY_DUPLEX, 0.9, (245, 66, 230), 2)

    #riazzadd end

    # cv2.putText(frame, "Left pupil:  " + str(left_pupil), (90, 130), cv2.FONT_HERSHEY_DUPLEX, 0.9, (245, 66, 230), 1)
    # cv2.putText(frame, "Right pupil: " + str(right_pupil), (90, 165), cv2.FONT_HERSHEY_DUPLEX, 0.9, (245, 66, 230), 1)
    cv2.imshow("IVP Team Bravo", frame)

    key = cv2.waitKey(1)
    if key == 27:
        break

    if cv2.waitKey(1) == 27:
        break

# cap.release()
cv2.destroyAllWindows()
