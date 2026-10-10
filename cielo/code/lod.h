#ifndef LOD_H
#define LOD_H

#include "HandmadeMath.h"

#define MAX_LODS 16
#define Z_FAR 5000.0f

#define LEFT   2
#define RIGHT  2
#define TOP    2
#define BOTTOM 2

typedef struct PatchLod PatchLod;
struct PatchLod
{
  int core;
  int left;
  int right;
  int top;
  int bottom;
};

typedef struct SingleLodInfo SingleLodInfo;
struct SingleLodInfo
{
  int start;
  int count;
};

typedef struct LodInfo LodInfo;
struct LodInfo
{
  SingleLodInfo info [LEFT][RIGHT][TOP][BOTTOM];
};

struct BaseTerrain;

void  calculateMaxLOD(struct BaseTerrain* terrain);
int   initLod(struct BaseTerrain* terrain);
void  destroyLod(struct BaseTerrain* terrain);
void  calculateLODRegions(struct BaseTerrain* terrain);
int   distanceToLOD(struct BaseTerrain* terrain, float distance);
void  printLODMap(struct BaseTerrain* terrain);
void  updateLOD(struct BaseTerrain* terrain, HMM_Vec3* pos);
void  updateLodMapPass1(struct BaseTerrain* terrain, HMM_Vec3* pos);
void  updateLodMapPass2(struct BaseTerrain* terrain);

#endif // LOD_H
