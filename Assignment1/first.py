from OpenGL.GL import *
import glfw

glfw.init()

# Create window with dimensions 1280px by 1000px and title "Hello World"
window = glfw.create_window(1280, 1000, "Hello World", None, None)

glfw.make_context_current(window)

while not glfw.window_should_close(window):
    glfw.poll_events()
    glClear(GL_COLOR_BUFFER_BIT)

    # Sets active colour to green
    glColor3f(0.0, 1.0, 0.0)
    
    # Draws triangle in immediate mode with outlined vertices
    glBegin(GL_TRIANGLES)
    glVertex2f(0.0, 0.5)
    glVertex2f(0.5, -0.25)
    glVertex2f(-0.5, -0.25)
    glEnd()

    glfw.swap_buffers(window)

glfw.terminate()
