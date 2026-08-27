#ifndef TERRAIN_TECHNIQUE_H
#define TERRAIN_TECHNIQUE_H

#include "technique.h"
#include "ogldev_math_3d.h"

typedef struct TerrainTechnique TerrainTechnique;
struct TerrainTechnique
{
    GLuint vpLoc;
};

bool initTerrainTech(void);
void setVP(const Matrix4f* VP);

#endif  /* TERRAIN_TECHNIQUE_H */
