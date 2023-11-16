#pragma once
#include <Windows.h>

struct KeyInput
{
	int Key;
	bool KeyDown;
};

static UCHAR clientKeyBuffer[3][256];