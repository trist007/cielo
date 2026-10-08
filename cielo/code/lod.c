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
updateLOD(HMM_Vec3* pos)
{
    updateLodMapPass1(pos);
    updateLodMapPass2(pos);
}

void
updateLodMapPass1(struct BaseTerrain* terrain, HMM_Vec3* pos)
{
    int CenterStep = terrain->patchSize / 2;

    for (int LodMapZ = 0 ; LodMapZ < terrain->numPatchesZ ; LodMapZ++) {
        for (int LodMapX = 0 ; LodMapX < terrain->numPatchesX ; LodMapX++) {
            int x = LodMapX * (terrain->patchSize - 1) + CenterStep;
            int z = LodMapZ * (terrain->patchSize - 1) + CenterStep;

            HMM_Vec3 PatchCenter = (HMM_Vec3){x * (float)terrain->worldScale, 0.0f, z * (float)terrain->worldScale};

            float distanceToCamera = CameraPos.Distance(PatchCenter);
            /*    float Distance(const Vector3f& v) const
                  {
                      float DistSquared = DistanceSquared(v);
                      float distance = sqrtf(DistSquared);
                      return distance;
                  }
                  
                  float DistanceSquared(const Vector3f& v) const
                  {
                      float delta_x = x - v.x;
                      float delta_y = y - v.y;
                      float delta_z = z - v.z;

                      float DistanceSquared = delta_x * delta_x + delta_y * delta_y + delta_z * delta_z;

                      return DistanceSquared;
                  }
            */

            int CoreLod = distanceToLOD(distanceToCamera);

            PatchLod* pPatchLOD = m_map.GetAddr(LodMapX, LodMapZ);
            pPatchLOD->Core = CoreLod;
        }
    }
}

void
updateLodMapPass2(struct BaseTerrain* terrain, HMM_Vec3* pos)
{
    int Step = terrain->patchSize / 2;

    for (int LodMapZ = 0 ; LodMapZ < terrain->numPatchesZ ; LodMapZ++) {
        for (int LodMapX = 0 ; LodMapX < terrain->numPatchesX ; LodMapX++) {
            int CoreLod = m_map.Get(LodMapX, LodMapZ).Core;

            int IndexLeft   = LodMapX;
            int IndexRight  = LodMapX;
            int IndexTop    = LodMapZ;
            int IndexBottom = LodMapZ;

            if (LodMapX > 0) {
                IndexLeft--;

                if (m_map.Get(IndexLeft, LodMapZ).Core > CoreLod) {
                    m_map.At(LodMapX, LodMapZ).Left = 1;
                } else {
                    m_map.At(LodMapX, LodMapZ).Left = 0;
                }
            }

            if (LodMapX < terrain->numPatchesX - 1) {
                IndexRight++;

                if (m_map.Get(IndexRight, LodMapZ).Core > CoreLod) {
                    m_map.At(LodMapX, LodMapZ).Right = 1;
                } else {
                    m_map.At(LodMapX, LodMapZ).Right = 0;
                }
            }

            if (LodMapZ > 0) {
                IndexBottom--;

                if (m_map.Get(LodMapX, IndexBottom).Core > CoreLod) {
                    m_map.At(LodMapX, LodMapZ).Bottom = 1;
                } else {
                    m_map.At(LodMapX, LodMapZ).Bottom = 0;
                }
            }

            if (LodMapZ < terrain->numPatchesZ - 1) {
                IndexTop++;

                if (m_map.Get(LodMapX, IndexTop).Core > CoreLod) {
                    m_map.At(LodMapX, LodMapZ).Top = 1;
                } else {
                    m_map.At(LodMapX, LodMapZ).Top = 0;
                }
            }
        }
    }
}
