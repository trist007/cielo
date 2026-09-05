#include "gl_load.h"

PFNGLPOLYGONMODEPROC             glPolygonMode;
PFNGLCREATEPROGRAMPROC           glCreateProgram;
PFNGLLINKPROGRAMPROC             glLinkProgram;
PFNGLGETPROGRAMIVPROC            glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
PFNGLVALIDATEPROGRAMPROC         glValidateProgram;
PFNGLDELETESHADERPROC            glDeleteShader;
PFNGLCLEARCOLORPROC              glClearColor;
PFNGLFRONTFACEPROC               glFrontFace;
PFNGLCULLFACEPROC                glCullFace;
PFNGLENABLEPROC                  glEnable;
PFNGLCLEARPROC                   glClear;
PFNGLUSEPROGRAMPROC              glUseProgram;
PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;
PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
PFNGLGENBUFFERSPROC              glGenBuffers;
PFNGLBINDBUFFERPROC              glBindBuffer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
PFNGLBUFFERDATAPROC              glBufferData;
PFNGLDRAWELEMENTSPROC            glDrawElements;
PFNGLDELETEBUFFERSPROC           glDeleteBuffers;
PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays;
PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
PFNGLCREATESHADERPROC            glCreateShader;
PFNGLSHADERSOURCEPROC            glShaderSource;
PFNGLCOMPILESHADERPROC           glCompileShader;
PFNGLGETSHADERIVPROC             glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
PFNGLATTACHSHADERPROC            glAttachShader;
PFNGLUNIFORM1FPROC               glUniform1f;


int gl_load_all(GL_get_proc_address func)
{
  glPolygonMode             = (PFNGLPOLYGONMODEPROC) func("glPolygonMode");
  glCreateProgram           = (PFNGLCREATEPROGRAMPROC) func("glCreateProgram");
  glLinkProgram             = (PFNGLLINKPROGRAMPROC) func("glLinkProgram");
  glGetProgramiv            = (PFNGLGETPROGRAMIVPROC) func("glGetProgramiv");
  glGetProgramInfoLog       = (PFNGLGETPROGRAMINFOLOGPROC) func("glGetProgramInfoLog");
  glValidateProgram         = (PFNGLVALIDATEPROGRAMPROC) func("glValidateProgram");
  glDeleteShader            = (PFNGLDELETESHADERPROC) func("glDeleteShader");
  glClearColor              = (PFNGLCLEARCOLORPROC) func("glClearColor");
  glFrontFace               = (PFNGLFRONTFACEPROC) func("glFrontFace");
  glCullFace                = (PFNGLCULLFACEPROC) func("glCullFace");
  glEnable                  = (PFNGLENABLEPROC) func("glEnable");
  glClear                   = (PFNGLCLEARPROC) func("glClear");
  glUseProgram              = (PFNGLUSEPROGRAMPROC) func("glUseProgram");
  glUniformMatrix4fv        = (PFNGLUNIFORMMATRIX4FVPROC) func("glUniformMatrix4fv");
  glGenVertexArrays         = (PFNGLGENVERTEXARRAYSPROC) func("glGenVertexArrays");
  glBindVertexArray         = (PFNGLBINDVERTEXARRAYPROC) func("glBindVertexArray");
  glGenBuffers              = (PFNGLGENBUFFERSPROC) func("glGenBuffers");
  glBindBuffer              = (PFNGLBINDBUFFERPROC) func("glBindBuffer");
  glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) func("glEnableVertexAttribArray");
  glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC) func("glVertexAttribPointer");
  glBufferData              = (PFNGLBUFFERDATAPROC) func("glBufferData");
  glDrawElements            = (PFNGLDRAWELEMENTSPROC) func("glDrawElements");
  glDeleteBuffers           = (PFNGLDELETEBUFFERSPROC) func("glDeleteBuffers");
  glDeleteVertexArrays      = (PFNGLDELETEVERTEXARRAYSPROC) func("glDeleteVertexArrays");
  glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC) func("glGetUniformLocation");
  glCreateShader            = (PFNGLCREATESHADERPROC) func("glCreateShader");
  glShaderSource            = (PFNGLSHADERSOURCEPROC) func("glShaderSource");
  glCompileShader           = (PFNGLCOMPILESHADERPROC) func("glCompileShader");
  glGetShaderiv             = (PFNGLGETSHADERIVPROC) func("glGetShaderiv");
  glGetShaderInfoLog        = (PFNGLGETSHADERINFOLOGPROC) func("glGetShaderInfoLog");
  glAttachShader            = (PFNGLATTACHSHADERPROC) func("glAttachShader");
  glUniform1f               = (PFNGLUNIFORM1FPROC) func("glUniform1f");
  
  return(1);
}
