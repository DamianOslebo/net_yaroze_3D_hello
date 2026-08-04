#ifndef __NY_LIGHT_H__
#define __NY_LIGHT_H__

#include <libps.h>

void InitLight(GsF_LIGHT *light, int nLight, int nX, int nY, int nZ, int nRed, int nGreen, int nBlue);
void InitAllLights();

#endif