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

void initTerrain(float WorldScale);

void Render(const BasicCamera& Camera);

void LoadFromFile(const char* pFilename);

float GetHeight(int x, int z);

float GetWorldScale(void);
