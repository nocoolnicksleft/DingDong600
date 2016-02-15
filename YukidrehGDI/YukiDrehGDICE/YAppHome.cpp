#include "stdafx.h"

#include "YAppHome.h"

#include "YukiDrehGDICE.h"


YAppHome::~YAppHome()
{

}

YAppHome::YAppHome (HWND phWnd, HINSTANCE phInstance) : YApplication(phWnd, phInstance)
{

}

bool YAppHome::Init() 
{

	return true;

}

bool YAppHome::Fini() 
{

	return true;

}

bool YAppHome::OnPaint(HDC dc)
{
	return true;

}


