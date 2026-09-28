#ifndef TEXTURE_H
#define TEXTURE_H

#include "platform.h"
#include "gl_load.h"

#include <stdbool.h>

#include "GL/glcorearb.h"


typedef struct Texture {
  GLuint textureObj;
  GLenum textureTarget;
  int width, height, bpp;

} Texture;

bool  textureLoad(struct Texture* texture, GLenum textureTarget, const char* filename);

#endif // TEXTURE_H
