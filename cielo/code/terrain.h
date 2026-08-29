#ifndef TERRAIN_H
#define TERRAIN_H

#include "triangle_list.h"


typedef struct BaseTerrain BaseTerrain;
struct BaseTerrain
{
 	int terrainSize;
	float worldScale;
  float *heightMap;
	TriangleList triangleList;
	TerrainTechnique terrainTech;
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



void initTerrain(float WorldScale);
void initBasicCamera(BasicCamera *gameCamera, PersProjInfo pers, HMM_Vec3 Pos, HMM_Vec3 Target, HMM_Vec3 Up);
GLint getUniformLocation(const char* pUniformName);

void Render(const BasicCamera& Camera);

void LoadFromFile(const char* pFilename);

float GetHeight(int x, int z);

float GetWorldScale(void);
