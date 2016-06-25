#ifndef _SYSTEM_CLASS_H_
#define _SYSTEM_CLASS_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <Windows.h>
#include "InputClass.h"
#include "GraphicsClass.h"
#include "TimeClass.h"

// This class initializes the system objects and
// runs the app every frame.
class SystemClass{
private:

	//Windows-related functions
	bool Frame(); 
	void InitializeWindows(int& screenWidth, int& screenHeight); 
	void ShutdownWindows(); 

	//mouse-related functions
	void SetInitialClickPositions(); 
	
	//functions related to fading effect
	void SetFadingEffects();

	//helper functions
	bool Contains(RECT rect, POINT pt);
	float Distance(float x1, float y1, float x2, float y2);
	float Distance(POINT p1, POINT p2);
	float Distance(XMFLOAT2 p1, POINT p2);
	float Distance(POINT p1, XMFLOAT2 p2);
	float Distance(XMFLOAT2 p1, XMFLOAT2 p2);
	bool ButtonLeftClicked(RECT rect);

	//objects and variables related to app system
	LPCWSTR m_applicationName;        //name of the application
	HINSTANCE m_hinstance;            //main instance
	int m_screenWidth;                //screen width of the window
    int m_screenHeight;               //screen height of the window
	HWND m_hwnd;                      //handle to the window
	InputClass* m_Input;              //InputClass object
	GraphicsClass* m_Graphics;        //GraphicsClass object
	TimeClass* m_Clock;               //TimeClass object
	
	//mouse-related variables
	POINT mousePt;           //current cursor position relative to client window
	POINT lClickPos;         //cursor position upon left clicking
	POINT rClickPos;         //cursor position upon right clicking

	//variables related to fading effects
	int fadeTimerID;         //ID of timer for the fading effect of screen
	bool fadingIn;           //whether the screen is fading into another game mode
	bool fadingOut;          //whether the screen is fading out of the current game mode

public:

	//initializers and destructors
	SystemClass();
	SystemClass(const SystemClass& other);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static SystemClass* ApplicationHandle = 0;
#endif