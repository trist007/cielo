#include "geomip.h"

static int
calcNumIndices(const BaseTerrain* terrain)
{
  int numQuads = (terrain->patchSize - 1) * (terrain->patchSize - 1);
  int numIndices = 0;
  const int maxPermutationsPerLevel = 16;   // on/off for each of the four sides
  const int indicesPerQuad = 6;             // two triangles

  for (int lod = 0; lod <= terrain->maxLOD; lod++)
  {
    printf("LOD %d: num quads %d\n", lod, numQuads);
    numIndices += numQuads * indicesPerQuad * maxPermutationsPerLevel;
    numQuads /= 4;
  }

  printf("Initial number of indices %d\n", numIndices);
  return numIndices;
}

static int
addTriangle(int index, GLuint* indices, GLuint v1, GLuint v2, GLuint v3)
{
  indices[index++] = v1;
  indices[index++] = v2;
  indices[index++] = v3;

  return index;
}

static int
createTriangleFan(const BaseTerrain* terrain, int index, GLuint* indices,
                  int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom,
                  int x, int z)
{
  int w = terrain->width;

  int stepLeft   = 1 << lodLeft;
  int stepRight  = 1 << lodRight;
  int stepTop    = 1 << lodTop;
  int stepBottom = 1 << lodBottom;
  int stepCenter = 1 << lodCore;

  GLuint indexCenter = (GLuint)((z + stepCenter) * w + x + stepCenter);

  // first up
  GLuint indexTemp1 = (GLuint)(z * w + x);
  GLuint indexTemp2 = (GLuint)((z + stepLeft) * w + x);

  index = addTriangle(index, indices, indexCenter, indexTemp1, indexTemp2);

  // second up
  if (lodLeft == lodCore)
  {
    indexTemp1 = indexTemp2;
    indexTemp2 += (GLuint)(stepLeft * w);

    index = addTriangle(index, indices, indexCenter, indexTemp1, indexTemp2);
  }

  // first right
  indexTemp1 = indexTemp2;
  indexTemp2 += (GLuint)stepTop;

  index = addTriangle(index, indices, indexCenter, indexTemp1, indexTemp2);

  // second right
  if (lodTop == lodCore)
  {
    indexTemp1 = indexTemp2;
    indexTemp2 += (GLuint)stepTop;

    index = addTriangle(index, indices, indexCenter, indexTemp1, indexTemp2);
  }

  // first down
  indexTemp1 = indexTemp2;
  indexTemp2 -= (GLuint)(stepRight * w);

  index = addTriangle(index, indices, indexCenter, indexTemp1, indexTemp2);

  // second down
  if (lodRight == lodCore)
  {
    indexTemp1 = indexTemp2;
    indexTemp2 -= (GLuint)(stepRight * w);

    index = addTriangle(index, indices, indexCenter, indexTemp1, indexTemp2);
  }

  // first left
  indexTemp1 = indexTemp2;
  indexTemp2 -= (GLuint)stepBottom;

  index = addTriangle(index, indices, indexCenter, indexTemp1, indexTemp2);

  // second left
  if (lodBottom == lodCore)
  {
    indexTemp1 = indexTemp2;
    indexTemp2 -= (GLuint)stepBottom;

    index = addTriangle(index, indices, indexCenter, indexTemp1, indexTemp2);
  }

  return index;
}

static int
initIndicesLODSingle(const BaseTerrain* terrain, int index, GLuint* indices,
                     int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom)
{
  int fanStep = 1 << (lodCore + 1);                // lod 0 -> 2, lod 1 -> 4, lod 2 -> 8 ...
  int endPos  = terrain->patchSize - 1 - fanStep;  // patch 5, step 2 -> 2; patch 9, step 2 -> 6

  for (int z = 0; z <= endPos; z += fanStep)
  {
    for (int x = 0; x <= endPos; x += fanStep)
    {
      int lLeft   = (x == 0)      ? lodLeft   : lodCore;
      int lRight  = (x == endPos) ? lodRight  : lodCore;
      int lBottom = (z == 0)      ? lodBottom : lodCore;
      int lTop    = (z == endPos) ? lodTop    : lodCore;

      index = createTriangleFan(terrain, index, indices,
                                lodCore, lLeft, lRight, lTop, lBottom, x, z);
    }
  }

  return index;
}

static int
initIndicesLOD(BaseTerrain* terrain, int index, GLuint* indices, int lod)
{
  int totalIndicesForLOD = 0;

  for (int l = 0; l < 2; l++)
  {
    for (int r = 0; r < 2; r++)
    {
      for (int t = 0; t < 2; t++)
      {
        for (int b = 0; b < 2; b++)
        {
          SingleLodInfo* info = &terrain->lodInfo[lod].info[l][r][t][b];

          info->start = index;
          index = initIndicesLODSingle(terrain, index, indices,
                                       lod, lod + l, lod + r, lod + t, lod + b);
          info->count = index - info->start;

          totalIndicesForLOD += info->count;
        }
      }
    }
  }

  printf("Total indices for LOD %d: %d\n", lod, totalIndicesForLOD);

  return index;
}

static int
initIndices(BaseTerrain* terrain, GLuint* indices)
{
  int index = 0;

  for (int lod = 0; lod <= terrain->maxLOD; lod++)
  {
    index = initIndicesLOD(terrain, index, indices, lod);
  }

  return index;
}

/* ------------------------------------------------------------------------
   Normals

   The shared index buffer is patch-relative, so the LOD 0 variant (which
   touches every vertex of a patch) is replayed once per patch with that
   patch's base vertex added.
   ------------------------------------------------------------------------ */

static void
calculateGeomipNormals(const BaseTerrain* terrain, Vertex* vertices, int numVertices,
                       const GLuint* indices)
{
  int numIndices = terrain->lodInfo[0].info[0][0][0][0].count;

  for (int z = 0; z < terrain->depth - 1; z += (terrain->patchSize - 1))
  {
    for (int x = 0; x < terrain->width - 1; x += (terrain->patchSize - 1))
    {
      int baseVertex = z * terrain->width + x;

      for (int i = 0; i < numIndices; i += 3)
      {
        unsigned int index0 = (unsigned int)baseVertex + indices[i];
        unsigned int index1 = (unsigned int)baseVertex + indices[i + 1];
        unsigned int index2 = (unsigned int)baseVertex + indices[i + 2];

        HMM_Vec3 v1 = Vec3_Subtract(vertices[index1].Position, vertices[index0].Position);
        HMM_Vec3 v2 = Vec3_Subtract(vertices[index2].Position, vertices[index0].Position);
        HMM_Vec3 normal = Vec3_CrossProduct(v1, v2);
        Vec3_Normalize(&normal);

        vertices[index0].Normal = Vec3_Add(vertices[index0].Normal, normal);
        vertices[index1].Normal = Vec3_Add(vertices[index1].Normal, normal);
        vertices[index2].Normal = Vec3_Add(vertices[index2].Normal, normal);
      }
    }
  }

  for (int i = 0; i < numVertices; i++)
  {
    Vec3_Normalize(&vertices[i].Normal);
  }
}

/* ------------------------------------------------------------------------
   Buffers (replaces the old populateBuffers)
   ------------------------------------------------------------------------ */

void
populateGeomipBuffers(BaseTerrain* terrain, int width, int depth, TriangleList* tl)
{
  int numVertices = width * depth;

  Vertex* vertices = (Vertex*)calloc(numVertices, sizeof(Vertex));  // calloc: normals start at zero
  if (!vertices)
  {
    fprintf(stderr, "Out of memory building terrain vertices\n");
    abort();
  }

  int index = 0;
  for (int z = 0; z < depth; z++)
  {
    for (int x = 0; x < width; x++)
    {
      Vertex* vertex = &vertices[index++];

      vertex->Position.X = (float)x * terrain->worldScale;
      vertex->Position.Y = array2Df_get(&terrain->heightMap, x, z);
      vertex->Position.Z = (float)z * terrain->worldScale;

      float size = (float)terrain->terrainSize;
      vertex->Texture.U = terrain->textureScale * (float)x / size;
      vertex->Texture.V = terrain->textureScale * (float)z / size;
    }
  }

  assert(index == numVertices);

  int maxIndices = calcNumIndices(terrain);
  GLuint* indices = (GLuint*)calloc(maxIndices, sizeof(GLuint));
  if (!indices)
  {
    fprintf(stderr, "Out of memory building terrain indices\n");
    abort();
  }

  int numIndices = initIndices(terrain, indices);
  printf("Final number of indices %d\n", numIndices);

  tl->numIndices = numIndices;

  calculateGeomipNormals(terrain, vertices, numVertices, indices);

  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, vertices, GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, indices, GL_STATIC_DRAW);

  free(vertices);
  free(indices);
}

void
createGeomipGrid(struct BaseTerrain* terrain)
{
  if ((terrain->width - 1) % (terrain->patchSize - 1) != 0)
  {
    int recommendedWidth = ((terrain->width - 1 + terrain->patchSize - 1) / (terrain->patchSize - 1)) * (terrain->patchSize - 1) + 1;
    printf("Width minus 1 (%d) must be divisible by patchSize minus 1 (%d)\n", terrain->width, terrain->patchSize);
    printf("Try using Width = %d\n", recommendedWidth);
    exit(0);
  }
  if ((terrain->depth - 1) % (terrain->patchSize - 1) != 0)
  {
    int recommendedDepth = ((terrain->depth - 1 + terrain->patchSize - 1) / (terrain->patchSize - 1)) * (terrain->patchSize - 1) + 1;
    printf("Depth minus 1 (%d) must be divisible by patchSize minus 1 (%d)\n", terrain->depth, terrain->patchSize);
    printf("Try using Depth = %d\n", recommendedDepth);
    exit(0);
  }
   
  if (terrain->patchSize < 3)
  {
    printf("The minimum patch size is 3 (%d)\n", terrain->patchSize);
    exit(0);
  }
  
  if (terrain->patchSize % 2 == 0)
  {
    printf("Patch size must be an odder number (%d)\n", terrain->patchSize);
    exit(0);
  }
  
  terrain->maxLOD = initLod(terrain);
  
  createGLState(&terrain->triangleList);
  // does with triangleListCreate did
  populateBuffers(terrain, terrain->width, terrain->depth, &terrain->triangleList);
   
  // Reset Bindings
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void
geomipGridRender(BaseTerrain* terrain, HMM_Vec3* cameraPos)
{
  updateLOD(terrain, cameraPos);
 
  glBindVertexArray(terrain->triangleList.VAO);
 
  for (int patchZ = 0; patchZ < terrain->numPatchesZ; patchZ++)
  {
    for (int patchX = 0; patchX < terrain->numPatchesX; patchX++)
    {
      PatchLod* plod = &terrain->lodMap[patchZ * terrain->numPatchesX + patchX];
      SingleLodInfo*  info = &terrain->lodInfo[plod->core].info[plod->left][plod->right][plod->top][plod->bottom];
 
      size_t baseIndex = sizeof(GLuint) * (size_t)info->start;
 
      int z = patchZ * (terrain->patchSize - 1);
      int x = patchX * (terrain->patchSize - 1);
      int baseVertex = z * terrain->width + x;
 
      glDrawElementsBaseVertex(GL_TRIANGLES, info->count, GL_UNSIGNED_INT,
                               (void*)baseIndex, baseVertex);
    }
  }
 
  glBindVertexArray(0);
}
