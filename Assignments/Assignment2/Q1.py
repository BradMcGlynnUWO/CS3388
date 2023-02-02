import glfw
from OpenGL.GL import *
# numpy is imported to easily translate the angle the dogs are at into
# a value that can be used to determine their coordinates
import numpy as np

# initializing script with variables for later use
glfw.init()
window = glfw.create_window(800, 800, "Q1 - Rotating Dogs", None, None)
glfw.make_context_current(window)
angles = [0, 45, 90, 135, 180, 225, 270, 315]
rotation_angle = 0

# generates useable dog_data
with open("dog.txt", "r") as f:
    raw_data = f.read().split()
    dog_data = []
    for i in raw_data:
        dog_data.append(float(i))


while not glfw.window_should_close(window):
    glfw.poll_events()
    # Clears screen to white
    glClearColor(1, 1, 1, 1)
    glClear(GL_COLOR_BUFFER_BIT)

    # Sets projection and modelview matrices
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    # sets viewing volume
    glOrtho(0, 60, 0, 60, -1, 1)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

    # Draws dogs colour black
    glColor3f(0, 0, 0)
    # this loop will give us 8 dogs, as there are 8 positions for them to go to
    for i in range(len(angles)):
        """finds the value of cos(angle[i]) in radians for x, and
        sin(angle[i]) in radians for y. This is done to get the correct
        x and y position for the dogs to rotate about around the circle."""
        x_pos = np.cos(np.radians(angles[i]))
        y_pos = np.sin(np.radians(angles[i]))

        """Translates, rotates, then draws the dogs to their positions on the circle.
        The circle is centered around (30, 30), and has a radius of 25. 30 + 25 * x_pos
        and 30 + 25 * y_pos gives the appropriate x and y coordinates to draw the
        dog data at."""
        glTranslatef(30 + 25 * x_pos, 30 + 25 * y_pos, 0)
        glRotatef(rotation_angle, 0, 0, 1)
        glBegin(GL_LINE_STRIP)

        # Gets vertex data from dog_data
        for j in range(0, len(dog_data), 2):
            dog_x = dog_data[j]
            dog_y = dog_data[j + 1]
            glVertex2f(dog_x, dog_y)
        glEnd()
        # Resets modelview matrix for next dog
        glLoadIdentity()
    # Rotates dogs 1 degree counterclockwise
    rotation_angle += 1
    glfw.swap_buffers(window)
glfw.terminate()
