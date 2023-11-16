#pragma once
#include <Windows.h>

struct KeyInput
{
	int Key;
	bool KeyDown;
	int PlayerNumber;
};

static UCHAR clientKeyBuffer[3][256];