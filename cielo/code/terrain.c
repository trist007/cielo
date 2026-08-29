#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/stat.h  >
#include <sys/types.h>

#include <GL/glew.h>

#include "terrain.h"

void terrainLoadHeightMapFile(BaseTerrain* terrain, const char* pFilename)
{
  int FileSize = 0;
  unsigned char* p = (unsigned char*)readBinaryFile(pFilename, &FileSize);

  if (FileSize % sizeof(float) != 0) {
    printf("%s:%d - '%s' does not contain a whole number of floats (size %d)\n",
           __FILE__, __LINE__, pFilename, FileSize);
    exit(0);
  }

  terrain->terrainSize = (int)sqrtf((float)FileSize / (float)sizeof(float));
  printf("Terrain size %d\n", terrain->terrainSize);

  if ((terrain->terrainSize * terrain->terrainSize) != (FileSize / (int)sizeof(float))) {
    printf("%s:%d - '%s' does not contain a square height map - size %d\n",
           __FILE__, __LINE__, pFilename, FileSize);
    exit(0);
  }

  terrain->heightMap.rows = terrain->terrainSize;
  terrain->heightMap.cols = terrain->terrainSize;
  terrain->heightMap.data = (float*)p;
}

void terrainLoadFromFile(BaseTerrain* terrain, const char* pFilename)
{
  terrainLoadHeightMapFile(terrain, pFilename);
  triangleListCreate(&terrain->triangleList, terrain->terrainSize, terrain->terrainSize, terrain);
}

HMM_Mat4 Camera_GetViewProjMatrix(const BasicCamera* camera)
{

  HMM_Vec3 eye    = camera->pos;
  HMM_Vec3 center = HMM_AddV3(camera->pos, camera->target);
  HMM_Vec3 up     = camera->up;

  HMM_Mat4 view = HMM_LookAt_RH(eye, center, up);

  return HMM_MulM4(camera->projection, view);
}

void terrainRender(BaseTerrain* terrain, const BasicCamera* camera)
{
  HMM_Mat4 VP = Camera_GetViewProjMatrix(camera);

  glUseProgram(terrain->shaderProg);
  glUniformMatrix4fv(terrain->VPLoc, 1, GL_FALSE, (const GLfloat*)&VP);

  triangleListRender(&terrain->triangleList);
}

void triangleListCreate(TriangleList* tl, int width, int depth, BaseTerrain* terrain)
{
  int numVertices = width * depth;
  int numTriangles = (width - 1) * (depth - 1) * 2;
  int numIndices = numTriangles * 3;

  Vertex* vertices = (Vertex*)malloc(numVertices * sizeof(Vertex));
  GLuint* indices  = (GLuint*)malloc(numIndices * sizeof(GLuint));

  if (!vertices || !indices) {
    fprintf(stderr, "Out of memory building terrain mesh\n");
    exit(0);
  }

  int index = 0;
  for (int z = 0; z < depth; z++) {
    for (int x = 0; x < width; x++) {
      Vertex* v = &vertices[index++];
      v->x = (float)x * terrain->worldScale;
      v->y = Array2D_Get(&terrain->heightMap, x, z);
      v->z = (float)z * terrain->worldScale;
    }
  }

  int idx = 0;

  for (int z = 0; z < depth - 1; z++) {
      for (int x = 0; x < width - 1; x++) {
          GLuint indexBottomLeft  = (GLuint)(z * width + x);
          GLuint indexTopLeft     = (GLuint)((z + 1) * width + x);
          GLuint indexTopRight    = (GLuint)((z + 1) * width + x + 1);
          GLuint indexBottomRight = (GLuint)(z * width + x + 1);

          indices[idx++] = indexBottomLeft;
          indices[idx++] = indexTopLeft;
          indices[idx++] = indexTopRight;

          indices[idx++] = indexBottomLeft;
          indices[idx++] = indexTopRight;
          indices[idx++] = indexBottomRight;
      }
  }

  tl->numIndices = numIndices;

  glGenVertexArrays(1, &tl->VAO);
  glBindVertexArray(tl->VAO);

  glGenBuffers(1, &tl->VB);
  glBindBuffer(GL_ARRAY_BUFFER, tl->VB);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vertex), vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &tl->IB);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tl->IB);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

  glBindVertexArray(0);

  free(vertices);
  free(indices);
}

void triangleListRender(TriangleList* tl)
{
  glBindVertexArray(tl->VAO);
  glDrawElements(GL_TRIANGLES, tl->numIndices, GL_UNSIGNED_INT, NULL);
  glBindVertexArray(0);
}

void triangleListDestroy(TriangleList* tl)
{
  glDeleteBuffers(1, &tl->VB);
  glDeleteBuffers(1, &tl->IB);
  glDeleteVertexArrays(1, &tl->VAO);
}

char* readFile(const char* file, int* size)
{
  FILE* file_ptr = fopen(file, "r");
  if (!file_ptr) {
    fprintf(stderr, "unable to open file ''%s': %s\n", file, strerror(errno));
    return(NULL);
  }

  struct _stat stat_buf;

  if (_stat(file, &stat_buf) != 0) {
    fprintf(stderr, "error getting file stats for '%s': %s\n", file, strerror(errno));
    fclose(file_ptr);
    return(NULL);
  }

  *size = (int)stat_buf.st_size;

  char* p = (char*)malloc(*size);
  assert(p);

  size_t bytes_read = fread(p, 1, *size, file_ptr);
  if ((int)bytes_read != *size) {
    fprintf(stderr, "read file error for '%s': %s\n", file, strerror(errno));
    return(NULL);
  }

  fclose(file_ptr);
  return(p);
}

char* readBinaryFile(const char* file, int* size)
{
  FILE* file_ptr = fopen(file, "rb");
  if (!file_ptr) {
    fprintf(stderr, "unable to open file ''%s': %s\n", file, strerror(errno));
    return(NULL);
  }

  struct _stat stat_buf;

  if (_stat(file, &stat_buf) != 0) {
    fprintf(stderr, "error getting file stats for '%s': %s\n", file, strerror(errno));
    fclose(file_ptr);
    return(NULL);
  }

  *size = (int)stat_buf.st_size;

  char* p = (char*)malloc(*size);
  assert(p);

  size_t bytes_read = fread(p, 1, *size, file_ptr);
  if ((int)bytes_read != *size) {
    fprintf(stderr, "read file error for '%s': %s\n", file, strerror(errno));
    return(NULL);
  }

  fclose(file_ptr);
  return(p);
}

void writeBinaryFile(const char* file, const void* data, int size)
{
  FILE *f = fopen(file, "wb");
  if (!f) {
    fprintf(stderr, "error opening '%s'\n", file);
    exit(0);
  }

  size_t bytes_written = fwrite(data, 1, size, f);
  if ((int)bytes_written != size) {
    fprintf(stderr, "error writing file '%s'\n", file);
    exit(0);
  }

  fclose(f);
}

GLint getUniformLocation(GameState* gamestate, const char* pUniformName)
{
  GLuint Location = glGetUniformLocation(gamestate->shaderProg, pUniformName);

  if (Location == INVALID_UNIFORM_LOCATION) {
    fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
  }

  return(Location);
}

bool AddShader(GameState* gamestate, GLenum ShaderType, const char* pFilename)
{

  FILE* file_ptr = NULL;

  errno_t err = fopen_s(&file_ptr, pFilename, "rb");
  if (err != 0 || file_ptr == NULL)
  {
    fprintf(stderr, "Error opening file error: %d\n", err);  
    return(false);
  }

  fseek(file_ptr, 0, SEEK_END);
  long fileSize = ftell(file_ptr);
  fseek(file_ptr, 0, SEEK_SET);
    
  char* buffer = (char*)malloc(fileSize + 1);
  size_t bytes_read = fread(buffer, 1, fileSize, file_ptr);
  buffer[fileSize] = '\0';
  fclose(file_ptr);

  GLuint ShaderObj = glCreateShader(ShaderType);

  if (ShaderObj == 0) {
    fprintf(stderr, "Error creating shader type %d\n", ShaderType);
    free(buffer);
    return(false);
  }

  if (gamestate->shaderCount >= MAX_SHADERS) {
    fprintf(stderr, "Too many shaders\n");
    free(buffer);
    return(false);
  }

  // Save the shader object - will be deleted in the destructor
  gamestate->shaderList[gamestate->shaderCount++] = ShaderObj;

  const GLchar* p[1] = { buffer };
  GLint Lengths[1] = { (GLint)bytes_read };

  glShaderSource(ShaderObj, 1, p, Lengths);

  glCompileShader(ShaderObj);

  free(buffer);

  GLint success;
  glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

  if (!success) {
    GLchar InfoLog[1024];
    glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
    fprintf(stderr, "Error compiling '%s': '%s'\n", pFilename, InfoLog);
    return(false);
  }

  glAttachShader(gamestate->shaderProg, ShaderObj);

  return true;
}

void cameraPrint(BasicCamera* camera)
{
    printf("Pos: (%.3f, %.3f, %.3f) Target: (%.3f, %.3f, %.3f) Up: (%.3f, %.3f, %.3f)\n",
           camera->pos.X, camera->pos.Y, camera->pos.Z,
           camera->target.X, camera->target.Y, camera->target.Z,
           camera->up.X, camera->up.Y, camera->up.Z);
}

void cameraOnMouse(BasicCamera* camera, int x, int y)
{
    /* delta from last known mouse position drives look direction */
    int deltaX = x - (int)camera->mousePos.X;
    int deltaY = y - (int)camera->mousePos.Y;

    camera->mousePos.X = (float)x;
    camera->mousePos.Y = (float)y;

    camera->AngleH += (float)deltaX / 20.0f;
    camera->AngleV += (float)deltaY / 20.0f;

    /* clamp vertical look so you can't flip past straight up/down */
    if (camera->AngleV > 90.0f)  camera->AngleV = 90.0f;
    if (camera->AngleV < -90.0f) camera->AngleV = -90.0f;

    /* edge flags - useful later if you want continuous turning while the
       mouse sits pinned at the window border */
    camera->OnLeftEdge  = (x <= 0);
    camera->OnRightEdge = (x >= camera->windowWidth - 1);
    camera->OnUpperEdge = (y <= 0);
    camera->OnLowerEdge = (y >= camera->windowHeight - 1);

    /* recompute target direction from the updated angles */
    float horRad = ToRadian(camera->AngleH);
    float verRad = ToRadian(camera->AngleV);

    camera->target.X = cosf(verRad) * sinf(horRad);
    camera->target.Y = sinf(verRad);
    camera->target.Z = cosf(verRad) * cosf(horRad);
}

void cameraOnKeyboard(BasicCamera* camera, int key)
{
    switch (key) {

    case GLFW_KEY_UP:
    case GLFW_KEY_W:
        camera->pos.X += camera->target.X * camera->speed;
        camera->pos.Y += camera->target.Y * camera->speed;
        camera->pos.Z += camera->target.Z * camera->speed;
        break;

    case GLFW_KEY_DOWN:
    case GLFW_KEY_S:
        camera->pos.X -= camera->target.X * camera->speed;
        camera->pos.Y -= camera->target.Y * camera->speed;
        camera->pos.Z -= camera->target.Z * camera->speed;
        break;

    case GLFW_KEY_LEFT:
    case GLFW_KEY_A: {
        HMM_Vec3 left = HMM_NormV3(HMM_Cross(camera->target, camera->up));
        camera->pos.X -= left.X * camera->speed;
        camera->pos.Y -= left.Y * camera->speed;
        camera->pos.Z -= left.Z * camera->speed;
        break;
    }

    case GLFW_KEY_RIGHT:
    case GLFW_KEY_D: {
        HMM_Vec3 right = HMM_NormV3(HMM_Cross(camera->target, camera->up));
        camera->pos.X += right.X * camera->speed;
        camera->pos.Y += right.Y * camera->speed;
        camera->pos.Z += right.Z * camera->speed;
        break;
    }

    default:
        break;
    }
}

GLFWwindow* glfw_init(int major_ver, int minor_ver, int width, int height, bool is_full_screen, const char* title)
{
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = is_full_screen ? glfwGetPrimaryMonitor() : NULL;
    GLFWwindow* window = NULL;

    // Ported fallback logic from C++
    if (major_ver == 0 && minor_ver == 0) {
        int versions[3][2] = { {4,6}, {4,3}, {3,3} };

        for (int i = 0; i < 3; i++) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, versions[i][0]);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, versions[i][1]);
            window = glfwCreateWindow(width, height, title, monitor, NULL);

            if (window) {
                break; // Successfully created a context
            }
        }
    } else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_ver);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_ver);
        window = glfwCreateWindow(width, height, title, monitor, NULL);
    }

    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(err));
        exit(1);
    }

    // Force V-Sync to prevent frame-rate lighting bugs
    glfwSwapInterval(1);
    // glEnable(GL_FRAMEBUFFER_SRGB);
    return window;
}
