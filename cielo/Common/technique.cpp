#include <stdio.h>
#include "GL/eglew.h"
#include <stdlib.h>
#include <string.h>

#include "ogldev_util.h"
#include "technique.h"

bool initTechnique()
{
  struct Technique tech = { 0 };

  if (tech.shaderProg) {
      glDeleteProgram(tech.shaderProg);
  }

  tech.shaderProg = glCreateProgram();

  if (tech.shaderProg == 0) {
      fprintf(stderr, "Error creating shader program\n");
      return false;
  }

  return true;
}

// Use this method to add shaders to the program. When finished - call finalize()
bool AddShader(ShaderObjList* list, GLenum ShaderType, const char* pFilename)
{

  FILE* file_ptr = NULL;

  errno_t err = fopen_s(&file_ptr, pFilename, "r");
  if (err != 0 || file_ptr == NULL)
  {
    fprintf(stderr, "Error opening file error: %s\n", err);  
    return(false);
  }

  char buffer[256];

  while (fgets(buffer, sizeof(buffer), file_ptr) != NULL);

  GLuint ShaderObj = glCreateShader(ShaderType);

  if (ShaderObj == 0) {
      fprintf(stderr, "Error creating shader type %d\n", ShaderType);
      return false;
  }

  // Save the shader object - will be deleted in the destructor
  m_shaderObjList.push_back(ShaderObj);


  const GLchar* p[1];
  p[0] = s.c_str();
  GLint Lengths[1] = { (GLint)s.size() };

  glShaderSource(ShaderObj, 1, p, Lengths);

  glCompileShader(ShaderObj);

  GLint success;
  glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

  if (!success) {
      GLchar InfoLog[1024];
      glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
      fprintf(stderr, "Error compiling '%s': '%s'\n", pFilename, InfoLog);
      return false;
  }

  glAttachShader(m_shaderProg, ShaderObj);

  return true;
}


// After all the shaders have been added to the program call this function
// to link and validate the program.
bool Technique::Finalize()
{
    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(m_shaderProg);

    glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &Success);

    if (Success == 0) {
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        return false;
    }

    glValidateProgram(m_shaderProg);

    glGetProgramiv(m_shaderProg, GL_VALIDATE_STATUS, &Success);

    if (Success == 0) {
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        return false;
    }

    // Delete the intermediate shader objects that have been added to the program
    for (ShaderObjList::iterator it = m_shaderObjList.begin() ; it != m_shaderObjList.end() ; it++) {
        glDeleteShader(*it);
    }

    m_shaderObjList.clear();

//    PrintUniformList();

    return GLCheckError();
}


void Technique::PrintUniformList()
{
    int Count = 0;
    glGetProgramiv(m_shaderProg, GL_ACTIVE_UNIFORMS, &Count);
    printf("Active Uniforms: %d\n", Count);

    GLint Size; 
    GLenum Type;
    const GLsizei BufSize = 16; 
    GLchar Name[BufSize];
    GLsizei Length; 

    for (int i = 0; i < Count; i++) {
        glGetActiveUniform(m_shaderProg, (GLuint)i, BufSize, &Length, &Size, &Type, Name);

        printf("Uniform #%d Type: %u Name: %s\n", i, Type, Name);
    }
}


void Technique::Enable()
{
    glUseProgram(m_shaderProg);
}


GLint Technique::GetUniformLocation(const char* pUniformName)
{
    GLuint Location = glGetUniformLocation(m_shaderProg, pUniformName);

    if (Location == INVALID_UNIFORM_LOCATION) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }

    return Location;
}
