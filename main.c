#include <stdio.h>
#include <stdlib.h>
#include <libps.h>
#include <string.h>
#include <pad.h>

/* Globals */
#define SCREEN_W   320
#define SCREEN_H   240

#define OT_LENGTH  12
#define PACKETMAX  196608//248000

#define MODEL_MEM_ADDRESS 0x80090000

GsOT            Wot[2];
GsOT_TAG        zSortTable[2][1<<OT_LENGTH];

PACKET          gpuPacketArea[2][PACKETMAX];

volatile u_char *bb0;
volatile u_char *bb1;

u_long          PadData;

GsF_LIGHT      flLights[2];
GsRVIEW2       view;

typedef struct {
    GsDOBJ2 gsObjectHandler;
    GsCOORDINATE2 gsObjectCoord;
} PlayerStructType0;

PlayerStructType0 playerStruct0;

/* Functions */
void InitGraphics(void);
void InitLight(GsF_LIGHT *light, int nLight, int nX, int nY, int nZ, int nRed, int nGreen, int nBlue);
void InitAllLights();
void InitializeView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPx, int nVPy, int nVPz, int nVRX, int nVRY, int nVRZ);
void AddModelToPlayer(PlayerStructType0 *player, int nX, int nY, int nZ, unsigned long *modelAddress);
void DrawPlayer(PlayerStructType0 *player, GsOT *ot);
static u_long PadRead(void);
void RenderFrame(void);

/* main() */

int main(void)
{
    FntLoad(960, 256);
    FntOpen(16, 16, 256, 200, 0, 512);
    GetPadBuf(&bb0, &bb1);
    InitGraphics();
    InitAllLights();
    InitializeView(&view, 250, 0, 3000, -500, 0, 0, 0, 0);
    AddModelToPlayer(&playerStruct0, 0, -200, 0, (unsigned long *)MODEL_MEM_ADDRESS);

    while(1)
    {
        PadData = PadRead();
        if (PadData & PADselect)
            break;
        RenderFrame();
    }
    ResetGraph(3);
    return 0;
}

void InitGraphics(void)
{
    /* Initialize GPU */
    GsInitGraph(
        SCREEN_W,
        SCREEN_H,
        GsINTER | GsOFSGPU,
        0,
        0
    );

    /* Create two frame buffers */
    GsDefDispBuff(
        0, 0,
        0, SCREEN_H
    );

    GsInit3D();

    Wot[0].length = OT_LENGTH;
    Wot[1].length = OT_LENGTH;

    Wot[0].org = zSortTable[0];
    Wot[1].org = zSortTable[1];

    GsClearOt(0, 0, &Wot[0]);
    GsClearOt(0, 0, &Wot[1]);
}

void RenderFrame(void)
{
    int currentBuffer = GsGetActiveBuff();
    GsSetWorkBase((PACKET*)gpuPacketArea[currentBuffer]);
    GsClearOt(0, 0, &Wot[currentBuffer]);
    DrawPlayer(&playerStruct0, &Wot[currentBuffer]);
    FntPrint("Hello World!\n");
    FntFlush(-1);
    VSync(0);
    GsSwapDispBuff();
    GsSortClear(0, 0, 0, &Wot[currentBuffer]);
    GsDrawOt(&Wot[currentBuffer]);
}

static u_long PadRead()
{
    return ~(
        (*(bb0 + 3)) |
        (*(bb0 + 2) << 8) |
        (*(bb1 + 3) << 16) |
        (*(bb1 + 2) << 24)
    );
    
}

void AddModelToPlayer(PlayerStructType0 *player, int nX, int nY, int nZ, unsigned long *modelAddress)
{
    modelAddress++; // Move to the next address for the model data
    GsMapModelingData(modelAddress); // Map the model data to its actual address in memory
    GsInitCoordinate2(WORLD, &player->gsObjectCoord); // Initialize the player's coordinate system);
    modelAddress++; modelAddress++; // Move to the next address for the model data
    
    // Link the model to the player's object handler
    // (u_long)modelAddress
    GsLinkObject4((u_long)modelAddress, 
        &player->gsObjectHandler, 
        0); 
    // Set the player's object handler coordinate to the player's coordinate system
    player->gsObjectHandler.coord2 = &player->gsObjectCoord; 
    player->gsObjectCoord.coord.t[0] = nX; // Set the player's X position
    player->gsObjectCoord.coord.t[1] = nY; // Set the player's Y position
    player->gsObjectCoord.coord.t[2] = nZ; // Set the player's Z position
    // Set the player's coordinate flag to 0 to indicate that the model will be drawn
    player->gsObjectCoord.flg = 0; 
}

void DrawPlayer(PlayerStructType0 *player, GsOT *ot)
{
    MATRIX templs, templw;
    GsGetLws(player->gsObjectHandler.coord2, &templw, &templs);
    GsSetLightMatrix(&templw);
    GsSetLsMatrix(&templs);
    GsSortObject4(&player->gsObjectHandler, ot, 4, (u_long *)getScratchAddr(0));
}

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

void InitializeView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPx, int nVPy, int nVPz, int nVRX, int nVRY, int nVRZ)
{
    GsSetProjection(nProjDist);
    view->rz = -nRZ;
    view->vpx = nVPx;
    view->vpy = nVPy;
    view->vpz = nVPz;
    view->vrx = nVRX;
    view->vry = nVRY;
    view->vrz = nVRZ;

    view->super = WORLD; // Set the super coordinate to WORLD
    // view->super = &playerStruct0.gsObjectCoord; // Set the super coordinate to the player's coordinate

    GsSetRefView2(view);
}
