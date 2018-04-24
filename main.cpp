#include <windows.h>
#include <iostream>

#pragma comment(lib, "user32.lib")

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch(uMsg){
		case WM_CREATE:// Create Message
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

int main(){
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW; //Window 
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = NULL;
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "log";

	if(!RegisterClass(&wndclass)){
		std::cerr<<"RegisterClass failed."<<std::endl;
		return 1;
	}

	HWND hwnd = CreateWindow("log", "log",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, NULL, NULL);

	ShowWindow(hwnd, 1);
	UpdateWindow(hwnd);

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
