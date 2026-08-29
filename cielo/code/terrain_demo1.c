#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "terrain.h"
#include "HandmadeMath.h"

static void KeyCallback         (GLFWwindow* window, int key,   int scancode, int action, int mods);
static void CursorPosCallback   (GLFWwindow* window, double x,  double y);
static void MouseButtonCallback (GLFWwindow* window, int Button, int Action, int Mode);

static HMM_Vec3 normalizeFloat3(HMM_Vec3 vector)
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

  gameCamera->AngleH = ToDegree(atan2f(gameCamera->target.Z, gameCamera->target.X)) - 90.0f;
  gameCamera->AngleV = -ToDegree(asinf(gameCamera->target.Y));

  gameCamera->speed = 10.0f; // or whatever default you want
  gameCamera->windowWidth  = WINDOW_WIDTH;
  gameCamera->windowHeight = WINDOW_HEIGHT;
  gameCamera->OnUpperEdge = false;
  gameCamera->OnLowerEdge = false;
  gameCamera->OnLeftEdge  = false;
  gameCamera->OnRightEdge = false;
  gameCamera->mousePos.X = (int)(WINDOW_WIDTH / 2);
  gameCamera->mousePos.Y = (int)(WINDOW_HEIGHT / 2);

  float aspect = pers.Width / pers.Height;
  gameCamera->projection = HMM_Perspective_RH_ZO(HMM_AngleDeg(pers.FOV), aspect, pers.zNear, pers.zFar);
}

// INPUT
static void PassiveMouseCB(GameState* gamestate, int x, int y)
{
    cameraOnMouse(&gamestate->gameCamera, x, y);
}
 
static void KeyboardCB(GameState* gamestate, unsigned int key, int state)
{
    if (state == GLFW_PRESS) {
 
        switch (key) {
 
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwDestroyWindow(gamestate->window);
            glfwTerminate();
            exit(0);
 
        case GLFW_KEY_C:
            cameraPrint(&gamestate->gameCamera);
            break;
 
        case GLFW_KEY_W:
            gamestate->isWireframe = !gamestate->isWireframe;
 
            if (gamestate->isWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
 
            break;
        }
    }
 
    cameraOnKeyboard(&gamestate->gameCamera, key);
}
 
static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)scancode; (void)mods;
    GameState* gamestate = (GameState*)glfwGetWindowUserPointer(window);
    KeyboardCB(gamestate, (unsigned int)key, action);
}
 
static void CursorPosCallback(GLFWwindow* window, double x, double y)
{
    GameState* gamestate = (GameState*)glfwGetWindowUserPointer(window);
    PassiveMouseCB(gamestate, (int)x, (int)y);
}
 
static void MouseButtonCallback(GLFWwindow* window, int Button, int Action, int Mode)
{
    (void)Mode;
    GameState* gamestate = (GameState*)glfwGetWindowUserPointer(window);
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    /* If you have a dedicated mouse-button handler, call it here, e.g.:
       Camera_OnMouseButton(gamestate->gameCamera, Button, Action, (int)x, (int)y); */
    (void)gamestate; (void)Button; (void)Action; (void)x; (void)y;
}

int main(int argc, char** argv)
{

  (void)argc; (void)argv;

  struct GameState* gamestate = (GameState*)malloc(sizeof(GameState));
  if (!gamestate) {
    fprintf(stderr, "unable to allocate memory for gamestate\n");
    return(1);
  }

  // create window
  int major_ver = 0;
  int minor_ver = 0;
  bool is_full_screen = false;

  gamestate->window = glfw_init(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Terrain Rendering - Demo 1");

  gamestate->shaderProg = glCreateProgram();

  if (!AddShader(gamestate, GL_VERTEX_SHADER, "terrain.vs")) {
      return(0);
  }

  if (!AddShader(gamestate, GL_FRAGMENT_SHADER, "terrain.fs")) {
      return(0);
  }

  GLint Success = 0;
  GLchar ErrorLog[1024] = { 0 };

  // Link shaders to program
  glLinkProgram(gamestate->shaderProg);
  // check if shaders linked successfully
  glGetProgramiv(gamestate->shaderProg, GL_LINK_STATUS, &Success);

  if (Success == 0) {
      glGetProgramInfoLog(gamestate->shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
      fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
      return(0);
  }

  // validate program
  glValidateProgram(gamestate->shaderProg);

  // check if validation went successfully
  glGetProgramiv(gamestate->shaderProg, GL_VALIDATE_STATUS, &Success);

  if (Success == 0) {
      glGetProgramInfoLog(gamestate->shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
      fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
      return(0);
  }

  // now that shaders are in memory we can delete
  for (int i = 0; i < gamestate->shaderCount; i++)
    glDeleteShader(gamestate->shaderList[i]);

  gamestate->shaderCount = 0;

  gamestate->VPLoc = getUniformLocation(gamestate, "gVP");

  if (gamestate->VPLoc == INVALID_UNIFORM_LOCATION) {
    free(gamestate);
    return(1);
  }

  gamestate->terrain.shaderProg = gamestate->shaderProg;
  gamestate->terrain.VPLoc      = gamestate->VPLoc;

  // glfw callbacks
  glfwSetWindowUserPointer(gamestate->window, gamestate);  /* lets callbacks find gamestate */

  glfwSetCursorPos(gamestate->window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

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

  gamestate->persProjInfo = (PersProjInfo){ FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

  initBasicCamera(&gamestate->gameCamera, gamestate->persProjInfo, Pos, Target, Up);

  // init terrain, init BaseTerrain
  float WorldScale = 4.0f;
  gamestate->terrain.worldScale = 4.0f;
  terrainLoadFromFile(&gamestate->terrain, "..\\data\\heightmap.save");

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(gamestate->window)) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      terrainRender(&gamestate->terrain, &gamestate->gameCamera);
      glfwSwapBuffers(gamestate->window);
      glfwPollEvents();
  }

  // shutdown

  free(gamestate);
  return(0);
}
