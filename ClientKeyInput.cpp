#include "ClientKeyInput.h"

bool clientKeyBuffer[3][256];

bool GetKeyBuffer(int PlayerNumber, int key)
{
    return clientKeyBuffer[PlayerNumber][key];
}

void SetKeyBuffer(int PlayerNumber, int key, bool bSet)
{
    clientKeyBuffer[PlayerNumber][key] = bSet;
}
