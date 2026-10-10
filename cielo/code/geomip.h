#ifndef GEOMIP_H
#define GEOMIP_H

#include "terrain.h"

static int calcNumIndices(const BaseTerrain* terrain);
static int addTriangle(int index, GLuint* indices, GLuint v1, GLuint v2, GLuint v3);
static int createTriangleFan(const BaseTerrain* terrain, int index, GLuint* indices,
                  int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom,
                  int x, int z);

static int
initIndicesLODSingle(const BaseTerrain* terrain, int index, GLuint* indices,
                     int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom);

static int initIndicesLOD(BaseTerrain* terrain, int index, GLuint* indices, int lod);

static int initIndices(BaseTerrain* terrain, GLuint* indices);

static void
calculateGeomipNormals(const BaseTerrain* terrain, Vertex* vertices, int numVertices,
                       const GLuint* indices);

void populateGeomipBuffers(BaseTerrain* terrain, int width, int depth, TriangleList* tl);

void  createGeomipGrid(struct BaseTerrain* terrain);
void geomipGridRender(BaseTerrain* terrain, HMM_Vec3* cameraPos);


#endif // GEOMIP_H
