#include "texture.h"
#include "GL/glcorearb.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "terrain.h"


void
loadSTBImage(STBImage* stb, const char* filename)
{
  stbi_set_flip_vertically_on_load(1);
  
  stb->imageData = stbi_load(filename, &stb->width, &stb->height, &stb->bpp, 0);
  
  if (!stb->imageData)
  {
    fprintf(stderr, "ERROR: can't load texture from '%s' - %s\n", filename, stbi_failure_reason());
    abort();
  }
  
  printf("Loaded '%s' - width %d, height %d, bpp %d\n", filename, stb->width, stb->height, stb->bpp);
}

void
// unLoadSTBImage(struct STBImage* stb)
unLoadSTBImage(STBImage* stb)
{
  printf("Unloading STB image\n");
  if (!stb->imageData)
  {
    fprintf(stderr, "ERROR: trying to unload a NULL image\n");
    abort();
  }
  
  stbi_image_free(stb->imageData);
  stb->imageData = NULL;
  stb->width     = 0;
  stb->height    = 0;
  stb->bpp       = 0;
}

void
loadTile(int* numTextureTiles, struct TextureTile* textureTiles, const char* filename)
{
  if (*numTextureTiles == MAX_TEXTURE_TILES)
  {
    fprintf(stderr, "ERROR: %s:%d: exceeded the maximum number of texture tiles with '%s'\n", __FILE__, __LINE__, filename);
    abort();
  }
  
  // textureTiles[*numTextureTiles]
  loadSTBImage(&textureTiles[*numTextureTiles].image, filename);
  *numTextureTiles += 1;
  
}

HMM_Vec3
getSTBImageColor(STBImage* stb, int x, int y)
{
  if (!stb->imageData)
  {
    fprintf(stderr, "%s:%d - trying to get the color but no texture was loaded\n", __FILE__, __LINE__);
    abort();
  }
  
  assert(stb->width > 0);
  assert(stb->height > 0);
  
  int wrappedX = x % stb->width;
  int wrappedY = y % stb->height;
  
  HMM_Vec3 Color;

  unsigned char* p = stb->imageData + (wrappedY * stb->width + wrappedX) *stb->bpp;
  Color.R = (float)p[0];
  Color.G = (float)p[1];
  Color.B = (float)p[2];
  
  return(Color);
}

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
  
  glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // highest quality but slowest
  glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
  
  glBindTexture(textureTarget, 0);
  stbi_image_free(data);
  
  printf("Loaded texture '%s' width %d, height %d, bpp %d\n", filename, texture->width, texture->height, texture->bpp);
  return(true);
  
}

void
generateTexture(struct GameState* gamestate, int textureSize, float minHeight, float maxHeight)
{
  if (gamestate->numTextureTiles == 0)
  {
    fprintf(stderr, "ERROR: %s:%d: no texture tiles loaded\n",__FILE__, __LINE__);
    abort();
  }
  
  calculateTextureRegions(gamestate, minHeight, maxHeight);
  
  int bpp = 3;
  int textureBytes = textureSize * textureSize * bpp;
  unsigned char* pTextureData = (unsigned char*)malloc(textureBytes);
  unsigned char* p = pTextureData;

  float heightMapToTextureRatio = (float)gamestate->terrain.terrainSize / (float)textureSize;

  printf("Height map to texture ratio: %f\n", heightMapToTextureRatio);

  for (int y = 0 ; y < textureSize ; y++) {
    for (int x = 0 ; x < textureSize ; x++) {

      float interpolatedHeight = getHeightInterpolated(&gamestate->terrain,
                                                       (float)x * heightMapToTextureRatio, (float)y * heightMapToTextureRatio);

      float red = 0.0f;
      float green = 0.0f;
      float blue = 0.0f;

      for (int Tile = 0 ; Tile < gamestate->numTextureTiles ; Tile++) {
        HMM_Vec3 color = getSTBImageColor(&gamestate->textureTiles[Tile].image, x, y);

        float blendFactor = regionPercent(gamestate, Tile, interpolatedHeight);

        red   += blendFactor * color.R;
        green += blendFactor * color.G;
        blue  += blendFactor * color.B;
      }

      if (red > 255.0f || green > 255.0f || blue > 255.0f) {
        printf("%d:%d: %f %f %f\n", y, x, red, green, blue);
        abort();
      }

      p[0] = (unsigned char)red;
      p[1] = (unsigned char)green;
      p[2] = (unsigned char)blue;

      p += 3;
    }
  }

  stbi_write_png("texture.png", textureSize, textureSize, bpp, pTextureData, textureSize * bpp);
  
  if (gamestate->numTextures == MAX_TEXTURES)
  {
    fprintf(stderr, "ERROR: max number %d of textures has been reached\n", MAX_TEXTURES);
    free(pTextureData);
    abort();
  }

  Texture* texture = &gamestate->texture[gamestate->numTextures];
  
  texture->textureTarget = GL_TEXTURE_2D;
  texture->height = textureSize;
  texture->width = textureSize;
  texture->bpp = bpp;
  texture->config = textureConfigDefault();
  strncpy_s(texture->filename, MAX_CHAR, "texture.png", _TRUNCATE);
  texture->filename[MAX_CHAR - 1] = '\0';

  bool isSRGB = false;
  
  textureLoadInternal(texture, pTextureData, isSRGB);
  
  // Activate texture and bind
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture->textureObj);
  
  gamestate->numTextures++;

  free(pTextureData);
}

void
calculateTextureRegions(struct GameState* gamestate, float minHeight, float maxHeight)
{
  float heightRange = maxHeight - minHeight;
  
  float rangePerTile = heightRange /  gamestate->numTextureTiles;
  float remainder = heightRange - rangePerTile * gamestate->numTextureTiles;
  
  if (remainder < 0.0f)
  {
    fprintf(stderr, "ERROR: %s:%d: negative remainder %f (num tiles %d range per tile %f)\n",
            __FILE__, __LINE__, remainder, gamestate->numTextureTiles, rangePerTile);
    abort();
  }
  
  float lastHeight = -1.0f;
  
  for (int i = 0; i < gamestate->numTextureTiles; i++)
  {
    gamestate->textureTiles[i].heightDesc.low = lastHeight + 1;
    lastHeight += rangePerTile;
    gamestate->textureTiles[i].heightDesc.optimal = lastHeight;
    gamestate->textureTiles[i].heightDesc.high = gamestate->textureTiles[i].heightDesc.optimal + rangePerTile;
    
    printf("Low: %f\t Optimal: %f\t High: %f\n", gamestate->textureTiles[i].heightDesc.low,
                                                 gamestate->textureTiles[i].heightDesc.optimal,
                                                 gamestate->textureTiles[i].heightDesc.high
    );

  }
}

float
regionPercent(GameState* gamestate, int tile, float height)
{
  float percent = 0.0f;

  if (height < gamestate->textureTiles[tile].heightDesc.low)
  {
    percent = 0.0f;
  }
  else if (height > gamestate->textureTiles[tile].heightDesc.high)
  {
    percent = 0.0f;
  }
  else if (height < gamestate->textureTiles[tile].heightDesc.optimal)
  {
    float nom = (float)height - (float)gamestate->textureTiles[tile].heightDesc.low;
    float denom = (float)gamestate->textureTiles[tile].heightDesc.optimal - (float)gamestate->textureTiles[tile].heightDesc.low;
    percent = nom / denom;
  }
  else if (height >= gamestate->textureTiles[tile].heightDesc.optimal)
  {
    float nom = (float)gamestate->textureTiles[tile].heightDesc.high - (float)height;
    float denom = (float)gamestate->textureTiles[tile].heightDesc.high - (float)gamestate->textureTiles[tile].heightDesc.optimal;
    percent = nom / denom;
  }
  else
  {
    printf("%s:%d - shouldn't get here! tile %d height %f\n", __FILE__, __LINE__, tile, height);
    abort();
  }
  
  if ((percent < 0.0f) || (percent > 1.0f))
  {
    printf("%s:%d - Invalid percent %f\n", __FILE__, __LINE__, percent);
    abort();
  }
  
  return(percent);
}

void
textureLoadInternal(Texture* texture, const void* pImageData, bool isSRGB)
{
  glGenTextures(1, &texture->textureObj);
  glBindTexture(texture->textureTarget, texture->textureObj);

  GLenum format = GL_RED;

  switch (texture->bpp)
  {
    case 1:
    {
      glTexImage2D(texture->textureTarget, 0, GL_RED, texture->width, texture->height,
                   0, GL_RED, GL_UNSIGNED_BYTE, pImageData);
      GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_RED };
      glTexParameteriv(texture->textureTarget, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
      break;
    }

    case 3:
      format = isSRGB ? GL_SRGB8 : GL_RGB8;
      glTexImage2D(texture->textureTarget, 0, format, texture->width, texture->height,
                   0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
      break;

    case 4:
      format = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
      glTexImage2D(texture->textureTarget, 0, format, texture->width, texture->height,
                   0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
      break;

    default:
      fprintf(stderr, "ERROR: %s:%d - unsupported image bpp %d\n", __FILE__, __LINE__, texture->bpp);
      abort();
  }

  glGenerateMipmap(texture->textureTarget);

  glTexParameteri(texture->textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(texture->textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(texture->textureTarget, GL_TEXTURE_WRAP_S, texture->config.wrapMode);
  glTexParameteri(texture->textureTarget, GL_TEXTURE_WRAP_T, texture->config.wrapMode);

  glBindTexture(texture->textureTarget, 0);
}

GLTextureConfig
textureConfigDefault(void)
{
  GLTextureConfig config;

  config.wrapMode   = GL_REPEAT;
  config.genMipmaps = true;

  return(config);
}
