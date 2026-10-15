#ifndef GEOMIP_H
#define GEOMIP_H

#include "HandmadeMath.h"

struct BaseTerrain;
struct TriangleList;


void populateGeomipBuffers(struct BaseTerrain* terrain, int width, int depth, struct TriangleList* tl);

void createGeomipGrid(struct BaseTerrain* terrain);
void geomipGridRender(struct BaseTerrain* terrain, HMM_Vec3* cameraPos, HMM_Mat4* viewProj);


#endif // GEOMIP_H
