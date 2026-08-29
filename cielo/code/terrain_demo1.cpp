#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "ogldev_glfw.h"
#include "terrain.h"
#include "HandmadeMath.h"
#include "math_3d.h"

#define WINDOW_WIDTH  1920.0f
#define WINDOW_HEIGHT 1080.0f

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void CursorPosCallback(GLFWwindow* window, double x, double y);
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode);

typedef struct BasicCamera BasicCamera;
struct BasicCamera
{
    HMM_Vec3 pos;
    HMM_Vec3 target;
    HMM_Vec3 up;

    float speed;
    int windowWidth;
    int windowHeight;

    float AngleH;
    float AngleV;

    bool OnUpperEdge;
    bool OnLowerEdge;
    bool OnLeftEdge;
    bool OnRightEdge;

    HMM_Vec2 mousePos;
    
    PersProjInfo persProjInfo;
    HMM_Mat4 projection;
};

typedef struct GameState GameState;
struct GameState
{
  GLFWwindow* window;
  BasicCamera* gameCamera;
  bool isWireframe;

  GLuint shaderProg;
  char *shaderList;
  int    shaderCount;
  GLuint VPLoc;
  PersProjInfo persProjInfo;

  struct BaseTerrain terrain;

};

GLint getUniformLocation(GameState* gamestate, const char* pUniformName)
{
    GLuint Location = glGetUniformLocation(gamestate->shaderProg, pUniformName);

    if (Location == INVALID_UNIFORM_LOCATION) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return(Location);
}

HMM_Vec3 normalizeFloat3(HMM_Vec3 vector)
{
  float length = sqrtf(vector.X*vector.X + vector.Y*vector.Y + vector.Z*vector.Z);

  vector.X = vector.X / length;
  vector.Y = vector.Y / length;
  vector.Z = vector.Z / length;

  return vector;
}

void initBasicCamera(BasicCamera *gameCamera, PersProjInfo pers, HMM_Vec3 Pos,
                     HMM_Vec3 Target, HMM_Vec3 Up)
{
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
  gameCamera->mousePos.X = int(WINDOW_WIDTH / 2);
  gameCamera->mousePos.Y = int(WINDOW_HEIGHT / 2);

  float aspect = pers.Width / pers.Height;
  gameCamera->projection = HMM_Perspective_RH_NO(HMM_AngleDeg(pers.FOV), aspect, pers.zNear, pers.zFar);
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

  size_t bytes_read = fread(buffer, 1, fileSize, file_ptr);

  GLuint ShaderObj = glCreateShader(ShaderType);

  if (ShaderObj == 0) {
      fprintf(stderr, "Error creating shader type %d\n", ShaderType);
      return false;
  }

  // Save the shader object - will be deleted in the destructor
  shaderObjList[shaderCount++] = ShaderObj;

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
    gameCamera->OnMouse(x, y);
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
            gameCamera->Print();
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

    gameCamera->OnKeyboard(key);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    KeyboardCB(key, action);
}


static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    PassiveMouseCB((int)x, (int)y);
}


static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode)
{
    double x, y;

    glfwGetCursorPos(window, &x, &y);

    MouseCB(Button, Action, (int)x, (int)y);
}


int main(int argc, char** argv)
{

  struct GameState* gamestate = (GameState*)malloc(sizeof(GameState));


  BaseTerrain* baseTerrain = (BaseTerrain*)malloc(sizeof(BaseTerrain));

  gamestate->shaderProg = glCreateProgram();

  if (!AddShader(gamestate->shaderList, GL_VERTEX_SHADER, "terrain.vs")) {
      return(0);
  }

  if (!AddShader(gamestate->shaderList, GL_FRAGMENT_SHADER, "terrain.fs")) {
      return(0);
  }

  GLint Success = 0;
  GLchar ErrorLog[1024] = { 0 };

  glLinkProgram(gamestate->shaderProg);

  glGetProgramiv(gamestate->shaderProg, GL_LINK_STATUS, &Success);

  if (Success == 0) {
      glGetProgramInfoLog(gamestate->shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
      fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
      return false;
  }

  glValidateProgram(gamestate->shaderProg);

  glGetProgramiv(gamestate->shaderProg, GL_VALIDATE_STATUS, &Success);

  if (Success == 0) {
      glGetProgramInfoLog(gamestate->shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
      fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
      return false;
  }

  for (int i = 0; i < gamestate->shaderCount; i++)
    glDeleteShader(gamestate->shaderList[i]);

  gamestate->shaderCount = 0;

  gamestate->VPLoc = GetUniformLocation("gVP");

  if (gamestate->VPLoc == INVALID_UNIFORM_LOCATION) {
      return false;
  }

  // create window
  int major_ver = 0;
  int minor_ver = 0;
  bool is_full_screen = false;
  gamestate->window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Terrain Rendering - Demo 1");

  glfwSetCursorPos(gamestate->window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

  // init callbacks
  glfwSetKeyCallback(gamestate->window, KeyCallback);
  glfwSetCursorPosCallback(gamestate->window, CursorPosCallback);
  glfwSetMouseButtonCallback(gamestate->window, MouseButtonCallback);

  // camera
  HMM_Vec3 Pos = {100.0f, 220.0f, -400.0f};
  HMM_Vec3 Target = {0.0f, -0.25f, 1.0f};
  HMM_Vec3 Up = {0.0, 1.0f, 0.0f};

  float FOV = 45.0f;
  float zNear = 0.1f;
  float zFar = 2000.0f;
  gamestate->persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

  gamestate->gameCamera = (BasicCamera*)malloc(sizeof(BasicCamera));
  initBasicCamera(gamestate->gameCamera, gamestate->persProjInfo, Pos, Target, Up);

  // init terrain, init BaseTerrain
  float WorldScale = 4.0f;
  gamestate->terrain.worldScale = 4.0f;
  initTerrain(gamestate->terrain);
  #ifdef _WIN32		
  gamestate->terrain = LoadFromFile("..\\data\\heightmap.save");
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
  free(gamestate);
  return 0;
}
