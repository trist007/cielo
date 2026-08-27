#ifndef TRIANGLE_LIST_H
#define TRIANGLE_LIST_H

#include <GL/glew.h>

#include "ogldev_math_3d.h"

// this header is included by terrain.h so we have a forward
// declaration for BaseTerrain.
//
struct Vertex
{
  Vector3f pos;
};

typedef struct BaseTerrain BaseTerrain;
struct BaseTerrain
{
  
  
};

typedef struct TriangleList TriangleList;
struct TriangleList
{
 	int m_width;
	int m_depth;
	GLuint m_vao;
	GLuint m_vb;
	GLuint m_ib;
  
};


void createTriangleList(int width, int depth, const BaseTerrain* pTerrain);
void render(void);

InitVertex(const BaseTerrain* pTerrain, int x, int z);
void CreateGLState();

void PopulateBuffers(const BaseTerrain* pTerrain);
void InitVertices(const BaseTerrain* pTerrain, std::vector<Vertex>& Vertices);
void InitIndices(std::vector<uint>& Indices);

#endif
