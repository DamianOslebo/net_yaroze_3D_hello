#include "pad.h"

volatile u_char *bb0;
volatile u_char *bb1;

void PadInit(void)
{
    GetPadBuf(&bb0, &bb1);
}

u_long PadRead()
{
    return ~(
        (*(bb0 + 3)) |
        (*(bb0 + 2) << 8) |
        (*(bb1 + 3) << 16) |
        (*(bb1 + 2) << 24)
    );
}