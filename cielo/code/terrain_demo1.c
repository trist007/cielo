#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "terrain.h"
#include "HandmadeMath.h"

static void processInput(PlatformWindow* window, GameState* gs)
{
    // Exit – continuous is fine, or use just_pressed
    if (platform_key_down(window, KEY_ESC) || platform_key_down(window, KEY_Q))
        platform_window_close(window);

    // One-shot actions
    if (platform_key_just_pressed(window, KEY_C))
        cameraPrint(&gs->gameCamera);

    if (platform_key_just_pressed(window, KEY_F)) {   // better key than W
        gs->isWireframe = !gs->isWireframe;
        glPolygonMode(GL_FRONT_AND_BACK, gs->isWireframe ? GL_LINE : GL_FILL);
    }

    // Continuous movement
    cameraOnKeyboard(&gs->gameCamera, window->keys);

    // Mouse
    cameraOnMouse(&gs->gameCamera, window->mouseX, window->mouseY);
}

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

int main(int argc, char** argv)
{

  (void)argc; (void)argv;

  struct GameState* gamestate = (GameState*)calloc(1, sizeof(GameState));
  if (!gamestate) {
    fprintf(stderr, "unable to allocate memory for gamestate\n");
    return(1);
  }

  // --- Platform init ---
  if (platform_init() < 0) {
    fprintf(stderr, "platform_init failed\n");
    return 1;
  }

  gamestate->window = platform_create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "Terrain Rendering");
  if (!gamestate->window) {
    fprintf(stderr, "Failed to create window\n");
    return 1;
  }

  // Load OpenGL functions (GLAD) AFTER the context is current
  if (!gladLoadGL((GLADloadfunc)platform_get_proc_address)) {
    fprintf(stderr, "Failed to load OpenGL functions\n");
    return 1;
  }

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

  // camera
  HMM_Vec3 Pos = {100.0f, 220.0f, -400.0f};
  HMM_Vec3 Target = {0.0f, -0.25f, 1.0f};
  HMM_Vec3 Up = {0.0, 1.0f, 0.0f};

  float FOV = 45.0f;
  float zNear = 0.1f;
  float zFar = 2000.0f;

  gamestate->persProjInfo = (PersProjInfo){ FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

  initBasicCamera(&gamestate->gameCamera, gamestate->persProjInfo, Pos, Target, Up);

  float WorldScale = 4.0f;
  gamestate->terrain.worldScale = 4.0f;

  // init terrain, init BaseTerrain
  // terrainLoadFromFile(&gamestate->terrain, "..\\data\\heightmap.save");
  int size = 256;
  int iterations = 500;
  float minHeight = 0.0f;
  float maxHeight = 300.0f;
  float filter = 0.5f;
  
  // NOTE(trist007): this will smoothen out peaks, at 0.5f the peaks are sharp, at 2.0f they are super round
  float roughness = 1.0f;

  gamestate->terrain.minHeightLoc = getUniformLocation(gamestate, "gMinHeight");
  gamestate->terrain.maxHeightLoc = getUniformLocation(gamestate, "gMaxHeight");

  // createFaultFormation(&gamestate->terrain, size, iterations, minHeight, maxHeight, filter);
  createMidpointDisplacement(&gamestate->terrain, size, roughness, minHeight, maxHeight);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  
  platform_set_user_data(gamestate->window, gamestate);

  while (!platform_window_should_close(gamestate->window))
  {
    
    platform_poll_events(gamestate->window);
    processInput(gamestate->window, gamestate);

    renderScene(&gamestate->terrain, &gamestate->gameCamera);
    platform_swap_buffers(gamestate->window);
  }

  // shutdown
  free(gamestate->terrain.heightMap.data);
  gamestate->terrain.heightMap.data = NULL;
  gamestate->terrain.heightMap.rows = 0;
  gamestate->terrain.heightMap.cols = 0;
  free(gamestate);
  return(0);
}
