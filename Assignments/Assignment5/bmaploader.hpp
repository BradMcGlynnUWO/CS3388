#ifndef BMPLOADER_H
#define BMPLOADER_H

#include <stdio.h>
#include <stdlib.h>

void loadARGB_BMP(const char* imagepath, unsigned char** data, unsigned int* width, unsigned int* height);

#endif // BMPLOADER_H
