#include <stdio.h>
#include <stdlib.h>
#include <libps.h>
#include <string.h>
#include "pad.h"
#include "ny_light.h"

/* Globals */
#define SCREEN_W   320
#define SCREEN_H   240

#define OT_LENGTH  12
#define PACKETMAX  196608//248000

#define MODEL_MEM_ADDRESS 0x80090000

GsOT            Wot[2];
GsOT_TAG        zSortTable[2][1<<OT_LENGTH];
PACKET          gpuPacketArea[2][PACKETMAX];

GsRVIEW2       view;

typedef struct {
    SVECTOR rotation;
    GsDOBJ2 gsObjectHandler;
    GsCOORDINATE2 gsObjectCoord;
} PlayerStructType0;

PlayerStructType0 playerStruct0;
u_long PadData;
u_char PLAYING = 1;
u_long vsyncInterval = 0;

extern GsF_LIGHT      flLights[2];

/* Functions */
void InitGraphics(void);
void InitializeStaticView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPx, int nVPy, int nVPz, int nVRX, int nVRY, int nVRZ);
void InitializeTrackerView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPx, int nVPy, int nVPz, int nVRX, int nVRY, int nVRZ);
void InitializePlayer(PlayerStructType0 *player, int nX, int nY, int nZ, unsigned long *modelAddress);
void AddModelToPlayer(PlayerStructType0 *player, int nX, int nY, int nZ, unsigned long *modelAddress);
void DrawPlayer(PlayerStructType0 *player, GsOT *ot);
void RenderFrame(void);
void ResetMatrix(short m[3][3]);
void RotateModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotationVector, int nX, int nY, int nZ);
void MoveModel(GsCOORDINATE2 *gsObjectCoord, int nX, int nY, int nZ);
void ProcessUserInput(void);
void AdvanceModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotationVector, int nD);

/* main() */
int main(void)
{
    FntLoad(960, 256);
    FntOpen(-96, -96, 256, 200, 0, 512);
    PadInit();
    InitGraphics();
    InitAllLights();
    InitializeStaticView(&view, 250, 0, 2000, -500, 0, 0, 0, 0);
    InitializePlayer(&playerStruct0, 0, -200, 0, (unsigned long *)MODEL_MEM_ADDRESS);

    while(PLAYING)
    {
        ProcessUserInput();
        RenderFrame();        
    }
    ResetGraph(0);
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

    DrawSync(0);
    vsyncInterval = VSync(0);
    FntPrint("VSync Interval: %d\n", vsyncInterval);
    FntFlush(-1);
    GsSwapDispBuff();

    GsSortClear(0, 0, 0, &Wot[currentBuffer]);
    GsDrawOt(&Wot[currentBuffer]);
}

void InitializePlayer(PlayerStructType0 *player, int nX, int nY, int nZ, unsigned long *modelAddress)
{
    player->rotation.vx = 0;
    player->rotation.vy = 0;
    player->rotation.vz = 0;
    AddModelToPlayer(player, nX, nY, nZ, modelAddress);
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

void InitializeStaticView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPx, int nVPy, int nVPz, int nVRX, int nVRY, int nVRZ)
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
    GsSetRefView2(view);
}

void InitializeTrackerView(GsRVIEW2 *view, int nProjDist, int nRZ, int nVPx, int nVPy, int nVPz, int nVRX, int nVRY, int nVRZ)
{
    GsSetProjection(nProjDist);
    view->rz = -nRZ;
    view->vpx = nVPx;
    view->vpy = nVPy;
    view->vpz = nVPz;
    view->vrx = nVRX;
    view->vry = nVRY;
    view->vrz = nVRZ;

    view->super = &playerStruct0.gsObjectCoord; // Set the super coordinate to the player's coordinate
    GsSetRefView2(view);
}

void MoveModel(GsCOORDINATE2 *gsObjectCoord, int nX, int nY, int nZ)
{
    gsObjectCoord->coord.t[0] += nX; // Move the model in the X direction
    gsObjectCoord->coord.t[1] += nY; // Move the model in the Y direction
    gsObjectCoord->coord.t[2] += nZ; // Move the model in the Z direction
    gsObjectCoord->flg = 0; // Set the flag to indicate that the model has been moved
}

void ResetMatrix(short m[3][3])
{
    m[0][0] = ONE; m[0][1] = 0;   m[0][2] = 0;
    m[1][0] = 0;   m[1][1] = ONE; m[1][2] = 0;
    m[2][0] = 0;   m[2][1] = 0;   m[2][2] = ONE;
}

void RotateModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotationVector, int nX, int nY, int nZ)
{
    MATRIX rotationMatrix;
    ResetMatrix(gsObjectCoord->coord.m); // Reset the rotation matrix to identity
    rotationVector->vx = (rotationVector->vx + nX) % ONE; // Update the rotation vector's X component
    rotationVector->vy = (rotationVector->vy + nY) % ONE; // Update the rotation vector's Y component
    rotationVector->vz = (rotationVector->vz + nZ) % ONE; // Update the rotation vector's Z component
    RotMatrix(rotationVector, &rotationMatrix);
    // Apply the rotation matrix to the object's coordinate system
    MulMatrix0(&gsObjectCoord->coord, &rotationMatrix, &gsObjectCoord->coord);
    gsObjectCoord->flg = 0; // Set the flag to indicate that the model has been rotated
}

void AdvanceModel(GsCOORDINATE2 *gsObjectCoord, SVECTOR *rotationVector, int nD)
{
    if (nD ==0) return; // No movement if nD is zero
    MATRIX maxTmp;
    SVECTOR startVector;
    SVECTOR currentDirection;

    startVector.vx = 0; // Start vector in the X direction
    startVector.vy = 0; // Start vector in the Y direction
    startVector.vz = ONE; // Start vector in the Z direction (forward/back
    RotMatrix(rotationVector, &maxTmp); // Create a rotation matrix based on the current rotation vector
    ApplyMatrixSV(&maxTmp, &startVector, &currentDirection); // Apply the rotation matrix to the start vector to get the current direction
    gsObjectCoord->coord.t[0] += (currentDirection.vx * nD)/ ONE; // Move the model in the X direction based on the current direction and distance
    gsObjectCoord->coord.t[1] += (currentDirection.vy * nD)/ ONE; // Move the model in the Y direction based on the current direction and distance
    gsObjectCoord->coord.t[2] += (currentDirection.vz * nD)/ ONE; // Move the model in the Z direction based on the current direction and distance

    gsObjectCoord->flg = 0; // Set the flag to indicate that the model has been moved
}

void ProcessUserInput(void)
{
    PadData = PadRead();
    if (PadData & PADselect)
        PLAYING = 0; // Exit the game loop if the select button is pressed
    
    if (PadData & PADLleft)
    {
        FntPrint("Left Arrow: Rotating Left\n");
        RotateModel(&playerStruct0.gsObjectCoord, &playerStruct0.rotation, 0, -64, 0); // Rotate left
    }
    if (PadData & PADLright)
    {
        FntPrint("Right Arrow: Rotating Right Lateral\n");
        RotateModel(&playerStruct0.gsObjectCoord, &playerStruct0.rotation, 0, 64, 0); // Rotate right
    }
    if (PadData & PADLup)
    {
        FntPrint("Up Arrow: Moving Forward Lateral\n");
        MoveModel(&playerStruct0.gsObjectCoord, 0, 0, 10); // Move up
    }
    if (PadData & PADLdown)
    {
        FntPrint("Down Arrow: Moving Backward\n");
        MoveModel(&playerStruct0.gsObjectCoord, 0, 0, -10); // Move down
    }
    if (PadData & PADRup)
    {
        FntPrint("Up Arrow: Moving Forward\n");
        AdvanceModel(&playerStruct0.gsObjectCoord, &playerStruct0.rotation, 16); // Move up
    }
    if (PadData & PADRdown)
    {
        FntPrint("Down Arrow: Moving Backward\n");
        AdvanceModel(&playerStruct0.gsObjectCoord, &playerStruct0.rotation, -16); // Move down
    }
    if (PadData & PADstart)
    {
        FntPrint("Start Button: Resetting Position\n");
        playerStruct0.gsObjectCoord.coord= GsIDMATRIX; // Reset the player's coordinate system to identity
        // Reset the player's rotation
        playerStruct0.rotation.vx = 0;
        playerStruct0.rotation.vy = 0; 
        playerStruct0.rotation.vz = 0;
        playerStruct0.gsObjectCoord.flg = 0;
    }
    if (PadData & PADL1)
    {
        FntPrint("Pad L1: Static View\n");
        InitializeStaticView(&view, 250, 0, -500, -1000, 0, 0, 0, 0);
    }
    if (PadData & PADR1)
    {
        FntPrint("Pad R1: Tracker View\n");
        InitializeTrackerView(&view, 250, 0, 0, -1000, -1000, 0, 0, 0);
    }
}