#include "platform.c"
#include <stdio.h>
#include <stdint.h>

void framebuffer_size_callback(PlatformWindow* window, int width, int height);
void processInput(PlatformWindow* window);

const uint32_t screenWidth = 800;
const uint32_T screenHeight = 600;

const char* vertexShaderSource = "#version 330 core\n"
  "layout (location = 0) in vec3 aPos;\n"
  "void main()\n"
  "{\n"
  "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
  "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
  "out vec4 FragColor;\n"
  "void main()\n"
  "{\n"
  "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
  "}\n\0";

int main()
{
  if (!platform_init())
  {
    fprintf(stderr, "platform_init failed\n");
    return(-1);
  }
  
  PlatformWindow* window = platform_create_window(screenWidth, screenHeight, "learnOpenGL");
  if (!window)
  {
    fprintf(stderr, "platform_create_window failed\n");
    return(-1);
  }
  
  if (!gladLoadGL((GLADloadfunc)platform_get_proc_address))
  {
    fprint(stderr, "failed to initialize GLAD\n");
    return (-1);
  }
  
  platform_set_framebuffer_size_callback(window, framebuffer_size_callback);
  
  while (!platform_window_should_close(window))
  {
    processInput(window);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    platform_swap_buffers(window);
    platform_poll_events(window);
  }
  
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shaderProgram);

  platform_terminate();
  return(0))
}

void processInput(PlatformWindow* window)
{
  if (platform_get_key(window, KEY_ESC) == KEY_PRESSED)
  {
    platform_window_close(window);
  }
}

void framebuffer_size_callback(PlatformWindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}
