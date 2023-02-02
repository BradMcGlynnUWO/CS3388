import random
# sys is used to take in command line arguments
import sys
from OpenGL.GL import *
import glfw


def frand() -> float:
    x = random.random()
    if x % 2 == 1:
        x *= -1.0
    return x


glfw.init()

# Get command line input in order to make window
n = int(sys.argv[1])
width, height = int(sys.argv[2]), int(sys.argv[3])
window = glfw.create_window(width, height, "Dot Plot", None, None)
glfw.make_context_current(window)
glOrtho(-1.1, 1.1, -1.1, 1.1, -1.0, 1.0)

# Sets  background colour to white and dot colour to black
glClearColor(1.0, 1.0, 1.0, 1.0)
glfw.poll_events()
glClear(GL_COLOR_BUFFER_BIT)
glColor3f(0, 0, 0)

# Initializes starting point and corner
p1 = [frand(), frand()]
c1 = random.randint(0, 3)
corners = [[-1, 1], [1, 1], [1, -1], [-1, -1]]

# glPointSize should be 2.0 here, but when testing the file the font size was too
# large to compare to the provided example, it didn't look correct. At 1.0, however,
# the output matches the assignment outlines output, hence why it is at 1.0 here
glPointSize(1.0)
glBegin(GL_POINTS)
for i in range(1, n+1):
    # Chooses a corner that is not diagonally opposite
    c2 = random.randint(0, 3)
    # will only return true if c2 = c1 + 2 or c2 = c1 - 1,
    # i.e. c2 is diagonal from c1, since % 4 only allows numbers 0-3
    while c2 % 4 == (c1 + 2) % 4:
        c2 = random.randint(0, 3)

    # Finds then draws halfway point between previous point and chosen corner
    x_pos = (p1[0]+corners[c2][0])/2
    y_pos = (p1[1]+corners[c2][1])/2
    p2 = [x_pos, y_pos]
    glVertex2f(p2[0], p2[1])

    # iterate p1 and c1
    p1 = p2
    c1 = c2
glEnd()
glfw.swap_buffers(window)

# repeats loop indefinitely
while not glfw.window_should_close(window):
    continue
glfw.terminate()
