#ifndef GL_LOAD_H
#define GL_LOAD_H

#include <GL/glcorearb.h>

typedef void* (*GL_get_proc_address)(const char* name);
// typedef GL_loader_proc (*GL_get_proc_address)(const char* name);

extern PFNGLACTIVETEXTUREPROC           glActiveTexture;
extern PFNGLATTACHSHADERPROC            glAttachShader;
extern PFNGLBINDBUFFERPROC              glBindBuffer;
extern PFNGLBINDTEXTUREPROC             glBindTexture;
extern PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
extern PFNGLBUFFERDATAPROC              glBufferData;
extern PFNGLCLEARCOLORPROC              glClearColor;
extern PFNGLCLEARPROC                   glClear;
extern PFNGLCOMPILESHADERPROC           glCompileShader;
extern PFNGLCREATEPROGRAMPROC           glCreateProgram;
extern PFNGLCREATESHADERPROC            glCreateShader;
extern PFNGLCULLFACEPROC                glCullFace;
extern PFNGLDELETEBUFFERSPROC           glDeleteBuffers;
extern PFNGLDELETESHADERPROC            glDeleteShader;
extern PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays;
extern PFNGLDRAWELEMENTSPROC            glDrawElements;
extern PFNGLENABLEPROC                  glEnable;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLFRONTFACEPROC               glFrontFace;
extern PFNGLGENBUFFERSPROC              glGenBuffers;
extern PFNGLGENERATEMIPMAPPROC          glGenerateMipmap;
extern PFNGLGENTEXTURESPROC             glGenTextures;
extern PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
extern PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
extern PFNGLGETPROGRAMIVPROC            glGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC             glGetShaderiv;
extern PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
extern PFNGLLINKPROGRAMPROC             glLinkProgram;
extern PFNGLPOLYGONMODEPROC             glPolygonMode;
extern PFNGLSHADERSOURCEPROC            glShaderSource;
extern PFNGLTEXIMAGE2DPROC              glTexImage2D;
extern PFNGLTEXPARAMETERIPROC           glTexParameteri;
extern PFNGLUNIFORM1FPROC               glUniform1f;
extern PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;
extern PFNGLUSEPROGRAMPROC              glUseProgram;
extern PFNGLVALIDATEPROGRAMPROC         glValidateProgram;
extern PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;

int gl_load_all(GL_get_proc_address func);


#endif // GL_LOAD_H
