#ifndef TERRAIN_H
#define TERRAIN_H

#include "texture.h"

#include "platform.h"
#include "gl_load.h"

#include "HandmadeMath.h"
#include <stdbool.h>
#include "array2df.h"

#include "GL/glcorearb.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080
#define MAX_CHAR      256
#define MAX_SHADERS   2
#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

#define PI 3.14159265358979323846f
#define powi(base,exp) (int)powf((float)(base), (float)(exp))
#define ToRadian(x) (float)(((x) * PI / 180.0f))
#define ToDegree(x) (float)(((x) * 180.0f / PI ))

typedef struct Vertex {
  HMM_Vec3 Position;
  HMM_Vec2 Texture;
  HMM_Vec3 Normal;
  
} Vertex;

typedef struct TriangleList TriangleList;
struct TriangleList
{
  /* VAO/VBO/index-buffer handles, vertex/index counts, etc. */
  GLuint VAO;
  GLuint VB;
  GLuint IB;
  int    numIndices;
};

typedef struct BaseTerrain BaseTerrain;
struct BaseTerrain
{
  GLuint shaderProg;
  
  GLuint VPLoc;
  GLuint reversedLightDirLoc;
  GLuint minHeightLoc;
  GLuint maxHeightLoc;
  GLuint tex0UnitLoc;
  GLuint tex1UnitLoc;
  GLuint tex2UnitLoc;
  GLuint tex3UnitLoc;
  GLuint tex0HeightLoc;
  GLuint tex1HeightLoc;
  GLuint tex2HeightLoc;
  GLuint tex3HeightLoc;

  float        worldScale;
  float        textureScale;
  float        minHeight;
  float        maxHeight;
  float        roughness;

  int          terrainSize;
  int          textureSize;
  int          depth;
  int          width;
  int          patchSize;
  Array2Df     heightMap;
  TriangleList triangleList;
  

  HMM_Vec3 ReversedLightDir;
};

typedef struct TerrainPoint TerrainPoint;
struct TerrainPoint
{
  int x;
  int z;
};

typedef struct FaultFormationTerrain FaultFormationTerrain;
struct FaultFormationTerrain
{
  GLuint       shaderProg;
  GLuint       VPLoc;
  float        worldScale;

  int          terrainSize;
  Array2Df     heightMap;
  TriangleList triangleList;

  TerrainPoint terrainPoint;
};

typedef struct MidpointDisplacementTerrain MidpointDisplacementTerrain;
struct MidpointDisplacementTerrain
{
  GLuint       shaderProg;
  GLuint       VPLoc;
  float        worldScale;

  int          terrainSize;
  Array2Df     heightMap;
  TriangleList triangleList;

  TerrainPoint terrainPoint;
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
  PlatformWindow*  window;
  BasicCamera gameCamera;
  bool isWireframe;

  GLuint shaderProg;
  GLuint shaderList[MAX_SHADERS];
  int    shaderCount;
  
  char   textureFile[4][256];
  struct Texture texture[4];
  int    numTextures;
  struct TextureTile textureTiles[MAX_TEXTURE_TILES];
  int    numTextureTiles;

  // GLuint VPLoc;
  PersProjInfo persProjInfo;
  struct BaseTerrain terrain;
};

void   initBasicCamera(BasicCamera *gameCamera, PersProjInfo pers, HMM_Vec3 Pos, HMM_Vec3 Target, HMM_Vec3 Up);
GLuint getUniformLocation(GameState* gamestate, const char* pUniformName);
void   terrainLoadHeightMapFile(BaseTerrain* terrain, const char* pFilename);
void   terrainLoadFromFile(BaseTerrain* terrain, const char* pFilename);
void   renderScene(BaseTerrain* terrain, const BasicCamera* camera);
void   createGLState(TriangleList* tl);
void   populateBuffers(BaseTerrain* terrain, int width, int depth, TriangleList* tl);

void  createFaultFormationInternal(Array2Df* heightMap, int terrainSize, int interations, float minHeight, float maxHeight, float filter);
void  createFaultFormation(struct BaseTerrain* terrain, int terrainSize, int interations, float minHeight, float maxHeight, float filter);
void  generateRandomTerrainPoints(int terrainSize, struct TerrainPoint* p1, struct TerrainPoint* p2);
int   areTerrainPointsEqual(struct TerrainPoint* p1, struct TerrainPoint* p2);
float getHeight(BaseTerrain* terrain, int x, int z);
float getHeightInterpolated(BaseTerrain* terrain, float x, float z);

void  createMidpointDisplacementF32(struct BaseTerrain* terrain, int terrainSize, float roughness);
void  createMidpointDisplacement(struct BaseTerrain* terrain, int terrainSize, float roughness, float minHeight, float maxHeight);
void  createGeomipGrid(struct BaseTerrain* terrain);
float randomFloatRange(float min, float max);
int   calcNextPowerOfTwo(int value);
int   isValuePowerOfTwo(int n);

void     triangleListCreate(TriangleList* tl, int width, int depth, BaseTerrain* terrain);
void     triangleListRender(TriangleList* tl);
void     triangleListDestroy(TriangleList* tl);
HMM_Mat4 Camera_GetViewProjMatrix(const BasicCamera* camera);
float    FIRFilterSinglePoint(Array2Df* heightMap, int x, int z, float prevVal, float filter);
void     applyFIRFilter(Array2Df* heightMap, int terrainSize, float filter);
void     calculateNormals(Vertex* vertices, int numVertices, GLuint* indices, int numIndices);
HMM_Vec3 Vec3_Subtract(HMM_Vec3 a, HMM_Vec3 b);
HMM_Vec3 Vec3_CrossProduct(HMM_Vec3 a, HMM_Vec3 b);
void     Vec3_Normalize(HMM_Vec3* normal);
HMM_Vec3 Vec3_Add(HMM_Vec3 a, HMM_Vec3 b);
HMM_Vec3 Vec3_Mul(HMM_Vec3 a, HMM_Vec3 b);
HMM_Vec3 Vec3_MulbyScalar(HMM_Vec3 a, float b);

void cameraPrint(BasicCamera* camera);
void cameraOnMouse(BasicCamera* camera, int x, int y);
void cameraOnKeyboard(BasicCamera* camera, const bool* keys);

char* readFile(const char* file, int* size);
void  writeBinaryFile(const char* pFilename, const void* pData, int size);
char* readBinaryFile(const char* file, int* size);
bool  AddShader(GameState* gamestate, GLenum ShaderType, const char* pFilename);

void diamondStep(int terrainSize, Array2Df* heightMap, int rectSize, float currentHeight);
void squareStep(int terrainSize, Array2Df* heightMap, int rectSize, float currentHeight);

#endif // TERRAIN_H
