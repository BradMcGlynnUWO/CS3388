import glfw
import OpenGL.GL as gl
# numpy is imported to easily translate the angle the dogs are at to a length to translate by
import numpy as np

# initializing useful variables for script
glfw.init()
window = glfw.create_window(800, 800, "Q1 - Rotating Dogs", None, None)
glfw.make_context_current(window)

with open("dog.txt", "r") as f:
    dog_data = [float(x) for x in f.read().split()]

angles = [0, 45, 90, 135, 180, 225, 270, 315]
rotation_angle = 0

while not glfw.window_should_close(window):
    glfw.poll_events()
    # Clears the screen to white
    gl.glClearColor(1, 1, 1, 1)
    gl.glClear(gl.GL_COLOR_BUFFER_BIT)

    # Sets projection and modelview matrices
    gl.glMatrixMode(gl.GL_PROJECTION)
    gl.glLoadIdentity()
    # sets viewing volume
    gl.glOrtho(0, 60, 0, 60, -1, 1)
    gl.glMatrixMode(gl.GL_MODELVIEW)
    gl.glLoadIdentity()

    # Draws dogs colour black
    gl.glColor3f(0, 0, 0)
    # this loop wil give us 8 dogs, as there are 8 positions for them to go to
    for i in range(len(angles)):
        """finds the value of cos(angle[i]) in radians for x, and
        sin(angle[i]) in radians for y. This is done to get the correct
        x and y position for the dogs to rotate about around the circle."""
        x_pos = np.cos(np.radians(angles[i]))
        y_pos = np.sin(np.radians(angles[i]))

        """Translates, rotates, then draws the dogs to their positions on the circle.
        The circle is centered around (30, 30), and has a radius of 25. 25 * x_pos
        and 25 * y_pos gives the appropriate x and y coordinates to draw the
        dog data at. """
        gl.glTranslatef(30 + 25 * x_pos, 30 + 25 * y_pos, 0)
        gl.glRotatef(rotation_angle, 0, 0, 1)
        gl.glBegin(gl.GL_LINE_STRIP)

        # Gets vertex data from dog_data
        for j in range(0, len(dog_data), 2):
            dog_x = dog_data[j]
            dog_y = dog_data[j + 1]
            gl.glVertex2f(dog_x, dog_y)
        gl.glEnd()
        # Resets modelview matrix for next dog
        gl.glLoadIdentity()
    # Rotates dogs 1 degree counterclockwise
    rotation_angle += 1
    glfw.swap_buffers(window)
glfw.terminate()


