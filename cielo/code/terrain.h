#ifndef TERRAIN_H
#define TERRAIN_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include "HandmadeMath.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080
#define MAX_SHADERS   2
#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

#define PI 3.14159265358979323846f
#define powi(base,exp) (int)powf((float)(base), (float)(exp))
#define ToRadian(x) (float)(((x) * PI / 180.0f))
#define ToDegree(x) (float)(((x) * 180.0f / PI ))


typedef struct Vertex {
    GLfloat x, y, z;
} Vertex;

typedef struct Array2Df Array2Df;
struct Array2Df {
  int    rows;
  int    cols;
  float* data;
};

static inline float Array2D_Get(const Array2Df* a, int x, int z)
{
    return a->data[z * a->cols + x];
}

typedef struct TriangleList {
    /* VAO/VBO/index-buffer handles, vertex/index counts, etc. */
  GLuint VAO;
  GLuint VB;
  GLuint IB;
  int    numIndices;
} TriangleList;

typedef struct BaseTerrain BaseTerrain;
struct BaseTerrain
{
  GLuint       shaderProg;
  GLint        VPLoc;
  float        worldScale;

  int          terrainSize;
  Array2Df     heightMap;
  TriangleList triangleList;
};

typedef struct PersProjInfo PersProjInfo;
struct PersProjInfo
{
  float FOV;
  float Width;
  float Height;
  float zNear;
  float zFar;
};

typedef struct BasicCamera BasicCamera;
struct BasicCamera
{
  HMM_Vec3 pos;
  HMM_Vec3 target;
  HMM_Vec3 up;

  float speed;
  int   windowWidth;
  int   windowHeight;

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
    GLFWwindow*  window;
    BasicCamera gameCamera;
    bool isWireframe;

    GLuint shaderProg;
    GLuint shaderList[MAX_SHADERS];
    int    shaderCount;

    GLuint VPLoc;
    PersProjInfo persProjInfo;

    struct BaseTerrain terrain;
};

void initBasicCamera(BasicCamera *gameCamera, PersProjInfo pers, HMM_Vec3 Pos, HMM_Vec3 Target, HMM_Vec3 Up);
GLint getUniformLocation(GameState* gamestate, const char* pUniformName);
void terrainLoadHeightMapFile(BaseTerrain* terrain, const char* pFilename);
void terrainLoadFromFile(BaseTerrain* terrain, const char* pFilename);
void terrainRender(BaseTerrain* terrain, const BasicCamera* camera);

void triangleListCreate(TriangleList* tl, int width, int depth, BaseTerrain* terrain);
void triangleListRender(TriangleList* tl);
void triangleListDestroy(TriangleList* tl);
HMM_Mat4 Camera_GetViewProjMatrix(const BasicCamera* camera);

void cameraPrint(BasicCamera* camera);
void cameraOnMouse(BasicCamera* camera, int x, int y);
void cameraOnKeyboard(BasicCamera* camera, int key);

char* readFile(const char* file, int* size);
void  writeBinaryFile(const char* pFilename, const void* pData, int size);
char* readBinaryFile(const char* file, int* size);
bool AddShader(GameState* gamestate, GLenum ShaderType, const char* pFilename);

GLFWwindow* glfw_init(int major_ver, int minor_ver, int width, int height, bool is_full_screen, const char* title);
#endif // TERRAIN_H
