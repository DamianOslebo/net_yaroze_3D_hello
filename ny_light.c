#include "ny_light.h"

GsF_LIGHT      flLights[2];

void InitAllLights()
{
    InitLight(&flLights[0], 0, -1, -1, -1, 255, 255, 255); // White light 1 
    InitLight(&flLights[1], 1,  1,  1,  1, 255, 255, 255);     // White light 2
    GsSetAmbient(0, 0, 0); // Set ambient light to black
    GsSetLightMode(0); // Enable lighting
}

void InitLight(GsF_LIGHT *light, int nLight, int nX, int nY, int nZ, int nRed, int nGreen, int nBlue)
{
    light->vx = nX;
    light->vy = nY;
    light->vz = nZ;
    light->r = nRed;
    light->g = nGreen;
    light->b = nBlue;

    GsSetFlatLight(nLight, light);
}   