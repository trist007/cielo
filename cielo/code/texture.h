#ifndef TEXTURE_H
#define TEXTURE_H

#include "platform.h"
#include "gl_load.h"

#define MAX_CHAR 256

#include <stdbool.h>

#include "GL/glcorearb.h"
#include "stb_image.h"
#include "HandmadeMath.h"

struct GameState;

#define MAX_TEXTURE_TILES 4
#define MAX_TEXTURES 8

typedef struct GLTextureConfig {
  GLenum wrapMode;
  bool genMipmaps;
} GLTextureConfig;

typedef struct Texture {
  GLuint textureObj;
  GLenum textureTarget;
  char filename[MAX_CHAR];
  int width, height, bpp;
  
  GLTextureConfig config;

} Texture;

typedef struct TextureHeightDesc {
  float low, optimal, high;
} TextureHeightDesc;

typedef struct STBImage {
  int width, height, bpp;
  unsigned char* imageData;
  char name[256];
} STBImage;

typedef struct TextureTile {
  STBImage image;
  TextureHeightDesc heightDesc;
} TextureTile;

struct GameState;

void     loadSTBImage(STBImage* stb, const char* filename);
void     unLoadSTBImage(STBImage* stb);
void     loadTile(int* numTextureTiles, struct TextureTile* textureTiles, const char* filename);
bool     textureLoad(struct Texture* texture, GLenum textureTarget, const char* filename);
void     textureLoadInternal(Texture* texture, const void* pImageData, bool isSRGB);
void     generateTexture(struct GameState* gamestate, int textureSize, float minHeight, float maxHeight);
void     loadInternal(const void* imageData, bool isRGB);
void     loadRaw(Texture* texture, int width, int height, int bpp, const void* imageData, bool isRGB);
void     calculateTextureRegions(struct GameState* gamestate, float minHeight, float maxHeight);
float    regionPercent(struct GameState* gamestate, int tile, float height);
HMM_Vec3 getSTBImageColor(STBImage* stb, int x, int y);
GLTextureConfig textureConfigDefault(void);


#endif // TEXTURE_H
