

#include "stdafx.h"

#include "YAppNavigation.h"

#include "YukiDrehGDICE.h"


YAppNavigation::~YAppNavigation()
{

}


YAppNavigation::YAppNavigation (HWND phWnd, HINSTANCE phInstance) : YApplication(phWnd, phInstance)
{
	currentApp = 0;
}

bool YAppNavigation::Init() 
{
	navigation[0] = new NavigationButton(0, hInstance, hWnd);
	navigation[0]->Init(IDB_BM_BUTTON1_ACTIVE, IDB_BM_BUTTON1_INACTIVE);
	navigation[0]->Activate();

	navigation[1] = new NavigationButton(1, hInstance, hWnd);
	navigation[1]->Init(IDB_BM_BUTTON2_ACTIVE, IDB_BM_BUTTON2_INACTIVE);

	navigation[2] = new NavigationButton(2, hInstance, hWnd);
	navigation[2]->Init(IDB_BM_BUTTON3_ACTIVE, IDB_BM_BUTTON3_INACTIVE);

	navigation[3] = new NavigationButton(3, hInstance, hWnd);
	navigation[3]->Init(IDB_BM_BUTTON4_ACTIVE, IDB_BM_BUTTON4_INACTIVE);

	navigation[4] = new NavigationButton(4, hInstance, hWnd);
	navigation[4]->Init(IDB_BM_BUTTON5_ACTIVE, IDB_BM_BUTTON5_INACTIVE);

	return true;

}

bool YAppNavigation::OnPaint(HDC dc)
{

	for (int i = 0; i < NAVIGATION_COUNT; i++) {

		navigation[i]->OnPaint(dc);

	}

	return true;

}

bool YAppNavigation::OnKeyDown(int virtualKeyCode)
{

	if (VK_F1 == virtualKeyCode) {

		navigation[currentApp]->Deactivate();

		currentApp = 0;

		navigation[currentApp]->Activate();

		return true;
		
	} else if (VK_F2 == virtualKeyCode) {

		navigation[currentApp]->Deactivate();

		currentApp = 1;

		navigation[currentApp]->Activate();

		return true;
		
	} else if (VK_F3 == virtualKeyCode) {

		navigation[currentApp]->Deactivate();

		currentApp = 2;

		navigation[currentApp]->Activate();

		return true;

	} else if (VK_F4 == virtualKeyCode) {

		navigation[currentApp]->Deactivate();

		currentApp = 3;

		navigation[currentApp]->Activate();

		return true;
		
	}

	return false;

}

int YAppNavigation::getCurrentApp()
{
	return currentApp;
}


bool YAppNavigation::OnLButtonDown(int x, int y)
{
	for (int i = 0; i < NAVIGATION_COUNT_ACTIVE; i++) {

		if (navigation[i]->IsHit(x,y)) {

			SetApplication(i);

			return true;

		}

	}

	return false;
}

bool YAppNavigation::OnLButtonUp(int x, int y)
{
	return false;
}

bool YAppNavigation::SetApplication(int id)
{
	if ( id != currentApp) {

		navigation[currentApp]->Deactivate();

		currentApp = id;

		navigation[currentApp]->Activate();

	}

	return true;
}

bool YAppNavigation::OnCommand(int identifier)
{
	switch (identifier)
	{
	case ID_ACC_F1:
			SetApplication(0);
			return true;
		break;

	case ID_ACC_F2:
			SetApplication(1);
			return true;
		break;

	case ID_ACC_F3:
			SetApplication(2);
			return true;
		break;

	case ID_ACC_F4:
			SetApplication(3);
			return true;
		break;

	case ID_ACC_F5:
		//SetApplication(4);
		return false;
		break;

	}

	return false;
}



