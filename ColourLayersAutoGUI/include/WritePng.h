#ifndef __WRITEPNG_H__
#define __WRITEPNG_H__

#ifdef __cplusplus
  extern "C" {
#endif

#ifdef WRITEPNG_EXPORTS
#define WRITEPNG_API __declspec(dllexport)
#else
#define WRITEPNG_API __declspec(dllimport)
#endif

//Функция для записи индексного пнг файла

WRITEPNG_API bool  PngIndexWriteImage(char* filename, const unsigned char* data, int step,
                               int width, int height);

#ifdef __cplusplus
  }	// extern "C"
#endif

#endif
