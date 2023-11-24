#pragma once
#include <Windows.h>

struct KeyInput
{
	int Key;
	bool KeyDown;
	int PlayerNumber;
};

bool GetKeyBuffer(int PlayerNumber, int key);
void SetKeyBuffer(int PlayerNumber, int key, bool bSet);
