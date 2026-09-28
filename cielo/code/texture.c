#include "texture.h"
#include "GL/glcorearb.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool
textureLoad(struct Texture* texture, GLenum textureTarget, const char* filename)
{
  texture->textureTarget = textureTarget;
  
  stbi_set_flip_vertically_on_load(1);
  
  unsigned char* data = stbi_load(filename, &texture->width, &texture->height, &texture->bpp, 0);
  
  if(!data)
  {
    fprintf(stderr, "ERROR: cannot load texture from '%s' - %s\n", filename, stbi_failure_reason());
    return(false);
  }
  
  glGenTextures(1, &texture->textureObj);
  glBindTexture(textureTarget, texture->textureObj);
  
  GLenum format = (texture->bpp == 4) ? GL_RGBA : (texture->bpp == 3) ? GL_RGB : GL_RED;
  
  glTexImage2D(textureTarget, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(textureTarget);
  
  glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  glBindTexture(textureTarget, 0);
  stbi_image_free(data);
  
  printf("Loaded texture '%s' width %d, height %d, bpp %d\n", filename, texture->width, texture->height, texture->bpp);
  return(true);
  
}
