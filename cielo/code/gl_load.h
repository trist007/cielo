#ifndef GL_LOAD_H
#define GL_LOAD_H

#include <GL/glcorearb.h>

typedef void* (*GL_get_proc_address)(const char* name);
// typedef GL_loader_proc (*GL_get_proc_address)(const char* name);

extern PFNGLPOLYGONMODEPROC             glPolygonMode;
extern PFNGLCREATEPROGRAMPROC           glCreateProgram;
extern PFNGLLINKPROGRAMPROC             glLinkProgram;
extern PFNGLGETPROGRAMIVPROC            glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
extern PFNGLVALIDATEPROGRAMPROC         glValidateProgram;
extern PFNGLDELETESHADERPROC            glDeleteShader;
extern PFNGLCLEARCOLORPROC              glClearColor;
extern PFNGLFRONTFACEPROC               glFrontFace;
extern PFNGLCULLFACEPROC                glCullFace;
extern PFNGLENABLEPROC                  glEnable;
extern PFNGLCLEARPROC                   glClear;
extern PFNGLUSEPROGRAMPROC              glUseProgram;
extern PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;
extern PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
extern PFNGLGENBUFFERSPROC              glGenBuffers;
extern PFNGLBINDBUFFERPROC              glBindBuffer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
extern PFNGLBUFFERDATAPROC              glBufferData;
extern PFNGLDRAWELEMENTSPROC            glDrawElements;
extern PFNGLDELETEBUFFERSPROC           glDeleteBuffers;
extern PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays;
extern PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
extern PFNGLCREATESHADERPROC            glCreateShader;
extern PFNGLSHADERSOURCEPROC            glShaderSource;
extern PFNGLCOMPILESHADERPROC           glCompileShader;
extern PFNGLGETSHADERIVPROC             glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
extern PFNGLATTACHSHADERPROC            glAttachShader;
extern PFNGLUNIFORM1FPROC               glUniform1f;

int gl_load_all(GL_get_proc_address func);


#endif // GL_LOAD_H
