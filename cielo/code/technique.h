#ifndef TECHNIQUE_H
#define TECHNIQUE_H

#include <GL/glew.h>

typedef struct Technique Technique;
struct Technique
{
  GLuint shaderProg;
  GLuint ShaderObjList* shaderObjList;
};

bool initTechnique(void);
void enable(void);
GLuint getProgram(void);
bool AddShader(GLenum ShaderType, const char* pFilename);
bool finalize(void);
GLint GetUniformLocation(const char* pUniformName);
void PrintUniformList(void);
};

// DEPRECATED
#ifdef FAIL_ON_MISSING_LOC                  
#define GET_UNIFORM_AND_CHECK(loc, name)    \
    loc = GetUniformLocation(name);         \
    if (loc == INVALID_UNIFORM_LOCATION)    \
        return false;                       
#else
#define GET_UNIFORM_AND_CHECK(loc, name)    \
    loc = GetUniformLocation(name);         
#endif

#define DEF_LOC_OLD(name) GLuint name = INVALID_UNIFORM_LOCATION

// NEW
#ifdef FAIL_ON_MISSING_LOC                  
#define GET_UNIFORM(name)    \
    m_##name##Loc = GetUniformLocation(#name); if (m_##name##Loc == INVALID_UNIFORM_LOCATION) return false
#else
#define GET_UNIFORM(name)    \
    m_##name##Loc = GetUniformLocation(#name)         
#endif

#define DEF_LOC(name) GLuint m_##name##Loc = -1

#endif
