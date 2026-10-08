#ifndef LOD_H
#define LOD_H

#include "HandmadeMath.h"

struct BaseTerrain;

int   initLod(struct BaseTerrain* terrain);
void  calculateMaxLOD(struct BaseTerrain* terrain);
void  calculateLODRegions(struct BaseTerrain* terrain);
int   distanceToLOD(struct BaseTerrain* terrain, float distance);
void  printLODMap(struct BaseTerrain* terrain);
void  updateLOD(HMM_Vec3* pos);
void  updateLodMapPass1(struct BaseTerrain* terrain, HMM_Vec3* pos);
void  updateLodMapPass2(struct BaseTerrain* terrain, HMM_Vec3* pos);

#endif // LOD_H
