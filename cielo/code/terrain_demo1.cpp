#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_glfw.h"
#include "ogldev_math_3d.h"
#include "terrain.h"

#define WINDOW_WIDTH  1920.0f
#define WINDOW_HEIGHT 1080.0f

GLuint g_shaderProg = 0;
GLuint g_shaderObjList[2];
int    g_shaderCount = 0;
GLuint g_VPLoc;

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);

typedef struct BasicCamera BasicCamera;
struct BasicCamera
{
    Vector3f pos;
    Vector3f target;
    Vector3f up;

    float speed;
    int windowWidth;
    int windowHeight;

    float AngleH;
    float AngleV;

    bool OnUpperEdge;
    bool OnLowerEdge;
    bool OnLeftEdge;
    bool OnRightEdge;

    Vector2i mousePos;
    
    PersProjInfo persProjInfo;
    Matrix4f projection;
};

Vector3f normalizeFloat3(Vector3f vector) {
  float length = sqrtf(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);

  vector.x = vector.x / length;
  vector.y = vector.y / length;
  vector.z = vector.z / length;

  return vector;
}

float ToDegree(float value) {
  return value = value * (180 / 3.14f);
}

void initBasicCamera(BasicCamera* gameCamera, persProjInfo pers, Vector3f Pos, Vector3f Target, Vector3f Up) {
  gameCamera->persProjInfo = pers;
  gameCamera->pos = Pos;
  gameCamera->target = normalizeFloat3(Target);
  gameCamera->up = normalizeFloat3(Up);

  gameCamera->AngleH = ToDegree(atan2f(gameCamera->target.z, gameCamera->target.x)) - 90.0f;
  gameCamera->AngleV = -ToDegree(asinf(gameCamera->target.y));

  gameCamera->speed = 10.0f; // or whatever default you want
  gameCamera->windowWidth  = WINDOW_WIDTH;
  gameCamera->windowHeight = WINDOW_HEIGHT;
  gameCamera->OnUpperEdge = false;
  gameCamera->OnLowerEdge = false;
  gameCamera->OnLeftEdge  = false;
  gameCamera->OnRightEdge = false;
  gameCamera->mousePos.x = WINDOW_WIDTH / 2;
  gameCamera->mousePos.y = WINDOW_HEIGHT / 2;

  Matrix4f_InitPersProjTransform(&gameCamera->m_projection, &pers);
}

bool AddShader(char* shaderList, GLenum ShaderType, const char* pFilename)
{

  FILE* file_ptr = NULL;

  errno_t err = fopen_s(&file_ptr, pFilename, "r");
  if (err != 0 || file_ptr == NULL)
  {
    fprintf(stderr, "Error opening file error: %d\n", err);  
    return(false);
  }

  char* buffer = NULL;
  long fileSize = 0;

  fseek(file_ptr, 0, SEEK_END);
  fileSize = ftell(file_ptr);
  fseek(file_ptr, 0, SEEK_SET);
    
  buffer = (char*)malloc(fileSize + 1);
  fread(buffer, 1, fileSize, file_ptr);
  buffer[fileSize] = '\0';
  fclose(file_ptr);

  while (fgets(buffer, sizeof(buffer), file_ptr) != NULL);

  GLuint ShaderObj = glCreateShader(ShaderType);

  if (ShaderObj == 0) {
      fprintf(stderr, "Error creating shader type %d\n", ShaderType);
      return false;
  }

  // Save the shader object - will be deleted in the destructor
  g_shaderObjList[g_shaderCount++] = ShaderObj;

  const GLchar* p[1];
  p[0] = buffer;
  GLint Lengths[1] = { (GLint)fileSize };

  glShaderSource(ShaderObj, 1, p, Lengths);

  glCompileShader(ShaderObj);

  free(buffer);

  GLint success;
  glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

  if (!success) {
      GLchar InfoLog[1024];
      glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
      fprintf(stderr, "Error compiling '%s': '%s'\n", pFilename, InfoLog);
      return false;
  }

  glAttachShader(g_shaderProg, ShaderObj);

  return true;
}


void PassiveMouseCB(int x, int y)
{
    m_pGameCamera->OnMouse(x, y);
}

void KeyboardCB(uint key, int state)
{
    if (state == GLFW_PRESS) {

        switch (key) {

        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(0);

        case GLFW_KEY_C:
            m_pGameCamera->Print();
            break;

        case GLFW_KEY_W:
            m_isWireframe = !m_isWireframe;

            if (m_isWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            break;
        }
    }

    m_pGameCamera->OnKeyboard(key);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    app->KeyboardCB(key, action);
}


static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    app->PassiveMouseCB((int)x, (int)y);
}


static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode)
{
    double x, y;

    glfwGetCursorPos(window, &x, &y);

    app->MouseCB(Button, Action, (int)x, (int)y);
}


int main(int argc, char** argv)
{

    GLFWwindow* window = NULL;
    BasicCamera* m_pGameCamera = NULL;
    bool m_isWireframe = false;

    BaseTerrain* baseTerrain = (BaseTerrain*)malloc(sizeof(BaseTerrain));

    g_shaderProg = glCreateProgram();

    if (!AddShader(GL_VERTEX_SHADER, "terrain.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "terrain.fs")) {
        return false;
    }

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(g_shaderProg);

    glGetProgramiv(g_shaderProg, GL_LINK_STATUS, &Success);

    if (Success == 0) {
        glGetProgramInfoLog(g_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        return false;
    }

    glValidateProgram(g_shaderProg);

    glGetProgramiv(g_shaderProg, GL_VALIDATE_STATUS, &Success);

    if (Success == 0) {
        glGetProgramInfoLog(g_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        return false;
    }

    for (int i = 0; i < g_shaderCount; i++)
      glDeleteShader(g_shaderObjList[i]);

    g_shaderCount = 0;

    g_VPLoc = GetUniformLocation("gVP");

    if (g_VPLoc == INVALID_UNIFORM_LOCATION) {
        return false;
    }

    // create window
    int major_ver = 0;
    int minor_ver = 0;
    bool is_full_screen = false;
    window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Terrain Rendering - Demo 1");

    glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

    // init callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // camera
    Vector3f Pos = {100.0f, 220.0f, -400.0f};
    Vector3f Target = {0.0f, -0.25f, 1.0f};
    Vector3f Up = {0.0, 1.0f, 0.0f};

    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 2000.0f;
    PersProjInfo persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

    BasicCamera* gameCamera = (BasicCamera*)malloc(sizeof(BasicCamera));
    initBasicCamera(gameCamera, persProjInfo, Pos, Target, Up);
    
    // init terrain, init BaseTerrain
    float WorldScale = 4.0f;
    m_terrain.InitTerrain(WorldScale);
    #ifdef _WIN32		
    m_terrain.LoadFromFile("..\\data\\heightmap.save");
#else 
    m_terrain.LoadFromFile("../data/heightmap.save");
#endif	

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_terrain.Render(*m_pGameCamera);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // shutdown

    free(baseTerrain);
    return 0;
}
