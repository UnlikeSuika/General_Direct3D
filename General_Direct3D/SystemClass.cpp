#include "SystemClass.h"

//SystemClass object initializer.
//Use SystemClass::Initialize to initialize the system.
SystemClass::SystemClass(){
	m_Input = 0;
	m_Graphics = 0;
	m_Clock = 0;
}

//SystemClass object initializer via SystemClass& argument.
//Do not use this initializer.
SystemClass::SystemClass(const SystemClass& other){}

//SystemClass destructor. Do not use this deleter.
//Use SystemClass::Shutdown instead.
SystemClass::~SystemClass(){}

//Initializes SystemClass variables
bool SystemClass::Initialize(){
	bool result;
	int screenWidth = 0;
	int screenHeight = 0;

	//Create a window for the application
	InitializeWindows(screenWidth, screenHeight);
	m_screenHeight = screenHeight;
	m_screenWidth = screenWidth;

	//Initialize InputClass object
	m_Input = new InputClass;
	if (!m_Input){
		MessageBox(m_hwnd, L"Could not create input object.", L"Error", MB_OK);
		return false;
	}
	m_Input->Initialize(m_hinstance, m_hwnd, screenWidth, screenHeight);

	//Initialize GraphicsClass object
	m_Graphics = new GraphicsClass;
	if (!m_Graphics){
		MessageBox(m_hwnd, L"Could not create graphics object.", L"Error", MB_OK);
		return false;
	}
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result){
		MessageBox(m_hwnd, L"Could not initialize graphics object.", L"Error", MB_OK);
		return false;
	}

	//Initialize TimeClass object
	m_Clock = new TimeClass;
	if (!m_Clock){
		MessageBox(m_hwnd, L"Could not create time object.", L"Error", MB_OK);
		return false;
	}
	m_Clock->SetFrameCount(0);
	
	//Set click positions to default -1
	lClickPos.x = -1;
	lClickPos.y = -1;
	rClickPos.x = -1;
	rClickPos.y = -1;

	//Not fading in or out
	fadingIn = false;
	fadingOut = false;

	return true;
}

//Shuts down SystemClass variables
void SystemClass::Shutdown(){

	//Shut down GraphicsClass object if present
	if (m_Graphics){
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	//Shut down InputClass object if present
	if (m_Input){
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}

	//Shut down TimeClass object if present
	if (m_Clock){
		delete m_Clock;
		m_Clock = 0;
	}

	//Shut down the application window
	ShutdownWindows();
}

//Receive, translate and dispatch system messages every frame
void SystemClass::Run(){
	MSG msg;
	bool done = false;
	ZeroMemory(&msg, sizeof(MSG));

	while (!done){

		//if system message is received
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//if system message is WM_QUIT, quit the game
		if (msg.message == WM_QUIT){
			done = true;
		}

		//otherwise, run the game during the given frame
		else{
			if (!Frame()){
				done = true;
			}
		}
	}
}

//Process the application every frame
bool SystemClass::Frame(){
	bool result;

	//Increase the frame count
	m_Clock->FrameIncrement();

	//Update mouse position every frame
	int mouseX, mouseY;
	m_Input->Frame(m_hwnd);
	m_Input->GetMouseLocation(mouseX, mouseY);
	mousePt.x = mouseX;
	mousePt.y = mouseY;

	//Set the left and right mouse click positions where applicable
	SetInitialClickPositions();

	//Start rendering the buffer
	m_Graphics->BeginRendering();

	//Set effects of screen fading
	SetFadingEffects();
	
	//Display what is currently in the buffer
	m_Graphics->EndRendering(m_screenWidth, m_screenHeight);

	//Update the previous key states to current key states
	m_Input->UpdatePrevKeyboardState();

	return true;
}

//Creates window for the application
void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight){
	ApplicationHandle = this;
	m_hinstance = GetModuleHandle(NULL);
	m_applicationName = L"UnlikeSuika's Programming Practice";

	//Setting window parameters
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	//Register window
	RegisterClassEx(&wc);

	//Set window size and styles
	screenWidth = 800;
	screenHeight = 600;
	RECT wr = { 0, 0, screenWidth, screenHeight };
	DWORD winStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	AdjustWindowRectEx(&wr, winStyle, FALSE, WS_EX_APPWINDOW);

	//Set window position
	int posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	int posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	//Create window
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, winStyle,
		posX, posY, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, m_hinstance, NULL);

	//Display window on foreground
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	//Display default cursor
	ShowCursor(true);
}

//Shuts down the application window
void SystemClass::ShutdownWindows(){

	//Display default cursor (just in case it was set not to be displayed)
	ShowCursor(true);

	//Destroy the application window object
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	//Unregister the window
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;
	ApplicationHandle = NULL;
}

//Update lClickPos and rClickPos when left/right mouse button is pressed/released
void SystemClass::SetInitialClickPositions(){
	
	//if left mouse button is just pressed this frame, set the click position to screen coord. of cursor
	if (m_Input->IsKeyJustPressed(VK_LBUTTON)){
		lClickPos.x = mousePt.x;
		lClickPos.y = mousePt.y;
	}

	//if left mouse button is released, reset the click position to (-1, -1)
	if (!m_Input->IsKeyDown(VK_LBUTTON) && !m_Input->IsKeyJustReleased(VK_LBUTTON)){
		lClickPos.x = -1;
		lClickPos.y = -1;
	}

	//if right mouse button is just pressed this frame, set the click position to screen coord. of cursor
	if (m_Input->IsKeyJustPressed(VK_RBUTTON)){
		rClickPos.x = mousePt.x;
		rClickPos.y = mousePt.y;
	}

	//if right mouse button is released, reset the click position to (-1, -1)
	if (!m_Input->IsKeyDown(VK_RBUTTON) && !m_Input->IsKeyJustReleased(VK_RBUTTON)){
		rClickPos.x = -1;
		rClickPos.y = -1;
	}
}

//Set the effect for screen fading into or out of game mode
void SystemClass::SetFadingEffects(){

	//if fading out of current mode
	if (fadingOut && (!fadingIn)){
	
		//if timer for fading out is already running
		if (m_Clock->IsTimerRunning(fadeTimerID)){
			m_Graphics->SetFadingEffect(1.0f*(1 - (float)m_Clock->TimeLeft(fadeTimerID) / 30));
	
			//if the timer has reached zero
			if (m_Clock->TimeLeft(fadeTimerID) == 0){
				fadingOut = false;
				fadingIn = true;
			}
		}
	
		//if timer for fading out has not yet started
		else{
			m_Graphics->StartFadingEffect();
			m_Clock->AddTimer(fadeTimerID, 30);
		}
	}

	// if fading into the next game mode
	else if ((!fadingOut) && fadingIn){
	
		//if the timer for fading in is running
		if (m_Clock->IsTimerRunning(fadeTimerID)){
			m_Graphics->SetFadingEffect(1.0f*(float)m_Clock->TimeLeft(fadeTimerID) / 30);
	
			//if the timer has reached zero
			if (m_Clock->TimeLeft(fadeTimerID) == 0){
				fadingIn = false;
				m_Graphics->StopFadingEffect();
				m_Clock->DeleteTimer(fadeTimerID);
			}
		}
	
		//if timer for fading in has not yet started
		else{
			m_Clock->SetTimer(fadeTimerID, 30);
		}
	}
}

//returns true if screen coordinates of the given point is contained within the given rectangle
bool SystemClass::Contains(RECT rect, POINT pt){
	if (pt.x > rect.left&&pt.x < rect.right&&pt.y > rect.top&&pt.y < rect.bottom){
		return true;
	}
	else{
		return false;
	}
}

//returns the distance between (x1, y1) and (x2, y2)
float SystemClass::Distance(float x1, float y1, float x2, float y2){
	return (float)sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
}

//returns the distance between p1 and p2
float SystemClass::Distance(POINT p1, POINT p2){
	return (float)sqrt(pow(p1.y - p2.y, 2) + pow(p1.x - p2.x, 2));
}

//returns the distance between p1 and p2
float SystemClass::Distance(XMFLOAT2 p1, POINT p2){
	return (float)sqrt(pow(p1.y - (float)p2.y, 2) + pow(p1.x - (float)p2.x, 2));
}

//returns the distance between p1 and p2
float SystemClass::Distance(POINT p1, XMFLOAT2 p2){
	return (float)sqrt(pow((float)p1.y - p2.y, 2) + pow((float)p1.x - p2.x, 2));
}

//returns the distance between p1 and p2
float SystemClass::Distance(XMFLOAT2 p1, XMFLOAT2 p2){
	return (float)sqrt(pow(p1.y - p2.y, 2) + pow(p1.x - p2.x, 2));
}

//returns true if a button is left-clicked
bool SystemClass::ButtonLeftClicked(RECT rect){
	return (m_Input->IsKeyJustReleased(VK_LBUTTON) && Contains(rect, lClickPos) && Contains(rect, mousePt));
}

//handles messages to the Windows application
LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
	case WM_KEYDOWN:
		m_Input->KeyDown((unsigned int)wParam);
		return 0;
	case WM_KEYUP:
		m_Input->KeyUp((unsigned int)wParam);
		return 0;
	case WM_LBUTTONDOWN:
		m_Input->KeyDown(VK_LBUTTON);
		SetCapture(m_hwnd);
		return 0;
	case WM_LBUTTONUP:
		m_Input->KeyUp(VK_LBUTTON);
		ReleaseCapture();
		return 0;
	case WM_RBUTTONDOWN:
		m_Input->KeyDown(VK_RBUTTON);
		return 0;
	case WM_RBUTTONUP:
		m_Input->KeyUp(VK_RBUTTON);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

//receives the translated messages, closes the application
//if WM_DESTROY or WM_CLOSE, handle message otherwise
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return ApplicationHandle->MessageHandler(hwnd, uMsg, wParam, lParam);
	}
}
