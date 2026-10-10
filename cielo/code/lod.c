struct BaseTerrain;

#include "lod.h"
#include "terrain.h"

void
calculateMaxLOD(struct BaseTerrain* terrain)
{
  int numSegments = terrain->patchSize - 1;
  if (ceilf(log2f((float)numSegments)) != floorf(log2f((float)numSegments)))
  {
    printf("The number of vertices in the patch minus one must be a power of two\n");
    printf("%f %f\n", ceilf(log2f((float)numSegments)), floorf(log2f((float)numSegments)));
    abort();
  }
  
  int patchSizeLog2 = (int)log2f((float)numSegments);
  printf("log2 of patch size %d is %d\n", terrain->patchSize, patchSizeLog2);
  terrain->maxLOD = patchSizeLog2 - 1;
}

int
initLod(struct BaseTerrain* terrain)
{
  calculateMaxLOD(terrain);
  
  terrain->lodMap = calloc((size_t)terrain->numPatchesX * terrain->numPatchesZ, sizeof(PatchLod)); 
  
  if (!terrain->lodMap)
  {
    fprintf(stderr, "ERROR: failed to allocate lodMap\n");
    abort();
  }
  
  terrain->lodRegions = calloc((size_t)terrain->maxLOD + 1, sizeof(int));
  
  if (!terrain->lodRegions)
  {
    fprintf(stderr, "ERROR: failed to allocate lodRegions\n");
    abort();
  }

  return(terrain->maxLOD);
}

void
calculateLODRegions(struct BaseTerrain* terrain)
{
  int sum = 0;

  for (int i = 0; i < terrain->maxLOD; i++) sum += (i + 1);
  
  printf("Sum %d\n", sum);
  
  float x = Z_FAR / (float)sum;
  
  int temp = 0;

  for (int i = 0; i <= terrain->maxLOD; i++)
  {
    int currentRange = (int)(x * (i + 1));

  }
}

int
distanceToLOD(struct BaseTerrain* terrain, float distance)
{
  int Lod = terrain->maxLOD;
  
  for (int i = 0; i < terrain->maxLOD; i++)
  {
    if (distance < terrain->lodRegions[i])
    {
      Lod = i;
      break;
    }
  }
  
  return(Lod);
}

void
printLODMap(struct BaseTerrain* terrain)
{
  for (int lodMapZ = terrain->numPatchesZ - 1; lodMapZ >= 0; lodMapZ--)
  {
    printf("%d: ", lodMapZ);
    for (int lodMapX = 0; lodMapX < terrain->numPatchesX; lodMapX++)
    {
      printf("%d ", terrain->lodMap[lodMapZ * terrain->numPatchesX + lodMapX].core);
    }
    printf("\n");
  }
}

void
updateLOD(struct BaseTerrain* terrain, HMM_Vec3* pos)
{
    updateLodMapPass1(terrain, pos);
    updateLodMapPass2(terrain);
}

void
updateLodMapPass1(struct BaseTerrain* terrain, HMM_Vec3* pos)
{
  int centerStep = terrain->patchSize / 2;

  for (int lodMapZ = 0 ; lodMapZ < terrain->numPatchesZ ; lodMapZ++) {
    for (int lodMapX = 0 ; lodMapX < terrain->numPatchesX ; lodMapX++) {
      int x = lodMapX * (terrain->patchSize - 1) + centerStep;
      int z = lodMapZ * (terrain->patchSize - 1) + centerStep;

      HMM_Vec3 patchCenter = HMM_V3(x * (float)terrain->worldScale, 0.0f, z * (float)terrain->worldScale);

      // float distanceToCamera = CameraPos.Distance(PatchCenter);
      float distanceToCamera = HMM_LenV3(HMM_SubV3(*pos, patchCenter));

      int coreLod = distanceToLOD(terrain, distanceToCamera);

      PatchLod* patchLod = &terrain->lodMap[lodMapZ * terrain->numPatchesX + lodMapX];
      patchLod->core = coreLod;
    }
  }
}

void
updateLodMapPass2(struct BaseTerrain* terrain)
{
  int nx = terrain->numPatchesX;
  int nz = terrain->numPatchesZ;

  for (int z = 0; z < nz; z++)
  {
    for (int x = 0; x < nx; x++)
    {
      PatchLod* patch = &terrain->lodMap[z * nx + x];
      int       core  = patch->core;
      
      patch->left   = (x > 0)      && terrain->lodMap[z * nx + (x - 1)].core > core;
      patch->right  = (x < nx - 1) && terrain->lodMap[z * nx + (x + 1)].core > core;
      patch->bottom = (z > 0)      && terrain->lodMap[(z - 1) * nx + x].core > core;
      patch->top    = (z < nz - 1) && terrain->lodMap[(z + 1) * nx + x].core > core;
    }
  }
}
