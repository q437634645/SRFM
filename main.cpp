#include <cassert>
#include <windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "msimg32.lib")

#define PI  3.1415926535898f

// define simple windows message process function
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class Vector4{
public:
	float x,y,z,w;
	Vector4():x(0), y(0), z(0), w(1){}
	Vector4(float x,float y,float z):x(x), y(y), z(z), w(1){}
};

class Matrix44{
public:
	float val[4][4];
	Matrix44(){
		memset(val, 0, 64);
	}
	float* operator[](const int &idx){
		return val[idx];
	}
	Matrix44 operator*(Matrix44 &m){
		Matrix44 res;
		for(int i=0;i<4;i++)
			for(int j=0;j<4;j++){
				res[i][j]=val[i][0]*m[0][j]+val[i][1]*m[1][j]+val[i][2]*m[2][j]+val[i][3]*m[3][j];
			}
		return res;
	}
};

typedef Vector4 Vec4;
typedef Matrix44 Mat;

class Device{
public:
	HWND hWindow;
	HDC hdcWindow;
	HDC hdcRenderer;
	HBITMAP hBitmap;
	unsigned char* pBuffer;
	int bufWidth;
	int bufHeight;
	Device(){
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
		}

		hWindow = CreateWindow("log", "log",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, CW_USEDEFAULT,
				NULL, NULL, NULL, NULL);

		ShowWindow(hWindow, 1);
		UpdateWindow(hWindow);

		hdcWindow = GetDC(hWindow);
		hdcRenderer = CreateCompatibleDC(hdcWindow);
		RECT rect;
		GetClientRect(hWindow, &rect);
		bufWidth = rect.right;
		bufHeight = rect.bottom;
		BITMAPINFO bi;
		memset(&bi, 0, sizeof(BITMAPINFO));
		bi.bmiHeader.biSize = sizeof(BITMAPINFO);
		bi.bmiHeader.biWidth = bufWidth;
		bi.bmiHeader.biHeight = bufHeight; // position = lower-left corner,negative = upper-left corner
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;
		hBitmap = CreateDIBSection(hdcRenderer, &bi, DIB_RGB_COLORS, (void**)&pBuffer, NULL, 0);
		SelectObject(hdcRenderer, hBitmap);
	}
	void Draw(){
		hdcWindow=GetDC(hWindow);
		BitBlt(hdcWindow, 0, 0, bufWidth, bufHeight, hdcRenderer, 0, 0, SRCCOPY);
	}
};

class Mesh{
public:
	std::vector<Vec4>vector_buffer;
	void AddTriangle(Vec4 a,Vec4 b,Vec4 c){
		vector_buffer.push_back(a);
		vector_buffer.push_back(b);
		vector_buffer.push_back(c);
	}
};

float interp(float a, float b, float t){
	return a * t + b * (1 - t);
}

Vec4 interp(Vec4 a,Vec4 b,float t){
	return Vec4(interp(a.x, b.x, t),
			interp(a.y, b.y, t),
			interp(a.z, b.z, t));
}

void Vec4MultMatrix(Vec4 &a,Vec4 &b,Mat &m);

class Renderer{
public:
	Renderer(Device *device)
		:bufWidth(device->bufWidth),
		bufHeight(device->bufHeight),
		pBuffer(device->pBuffer),
		device(device)
	{
		assert(device != NULL);
		zBuffer = new float[bufHeight * bufWidth];
	}
	void Update(){
		device->Draw();
	}
	void Draw(){
		int tolByte = bufHeight * bufWidth * 4;
		int countByte = 0;
		int writeByte;
		for(int i = 0;i < bufHeight;i += 100){
			writeByte = min(tolByte - countByte, bufWidth * 400);
			memset(pBuffer + countByte, i, writeByte);
			countByte += bufWidth * 400;
		}
	}
#define DrawPixel(x,y,r,g,b) \
	{pBuffer[(x)*4+(y)*bufWidth*4+1]=(r);\
	 pBuffer[(x)*4+(y)*bufWidth*4+2]=(g);\
	 pBuffer[(x)*4+(y)*bufWidth*4+3]=(b);}
	void DrawLine(Vec4 a,Vec4 b){
		int x1,y1,x2,y2;
		int tmp;
		x1=(int)a.x;y1=(int)a.y;
		x2=(int)b.x;y2=(int)b.y;
		if(x1==x2){
			for(int i=y1;i<=y2;i++){
				DrawPixel(x1,i,0,0,0);
			}
		}else if(y1==y2){
			int cot=0;
			for(int i=x1;i<=x2;i++){
				cot++;
				DrawPixel(i,y1,0,0,0);
			}
		}else{
			int dx=x1>x2?x1-x2:x2-x1;
			int dy=y1>y2?y1-y2:y2-y1;
			int dx2=2*dx;
			int dy2=2*dy;
			if(dx>=dy){
				if(x1>x2){tmp=x2;x2=x1;x1=tmp;tmp=y2;y2=y1;y1=tmp;}
				int fac=y1<y2?1:-1;
				int yn=y1;
				int p=dx;
				int dp=dy2-dx2;
				for(int i=x1;i<=x2;i++){
					DrawPixel(i,yn,0,0,0);
					if(p>=0)p+=dp;
					else p+=dy2;
					if(p>=0)yn+=fac;
				}
			}
			else{
				if(y1>y2){tmp=x2;x2=x1;x1=tmp;tmp=y2;y2=y1;y1=tmp;}
				int fac=x1<x2?1:-1;
				int xn=x1;
				int p=dy;
				int dp=dx2-dy2;
				for(int i=y1;i<=y2;i++){
					DrawPixel(xn,i,0,0,0);
					if(p>=0)p+=dp;
					else p+=dx2;
					if(p>=0)xn+=fac;
				}
			}
		}
	}
	void MapToDevice(Vec4 &a){
		float invw = 1.0f;
		if(a.w>1e-8||a.w<-1e-8)invw = 1/a.w;
		a.x*=invw;a.y*=invw;a.z*=invw;a.w=1.0f;
		a.x=(a.x+1.0f)*0.5f*bufWidth;
		a.y=(1.0f-a.y)*0.5f*bufHeight;
	}
	void DrawTriangle(Vec4 a,Vec4 b,Vec4 c){
		MapToDevice(a);
		MapToDevice(b);
		MapToDevice(c);
		DrawLine(a,b);
		DrawLine(b,c);
		DrawLine(c,a);
	}
	void DrawMesh(Mesh &m,Mat &r){
		int vertex_cot = m.vector_buffer.size();
		assert(vertex_cot%3 == 0);
		int triangle_cot = vertex_cot/3;
		Vec4 a,b,c;
		for(int i=0;i<triangle_cot;i++){
			Vec4MultMatrix(a,m.vector_buffer[i*3],r);
			Vec4MultMatrix(b,m.vector_buffer[i*3+1],r);
			Vec4MultMatrix(c,m.vector_buffer[i*3+2],r);
			DrawTriangle(a,b,c);
		}
	}
	void Clear(){
		memset(pBuffer,0xff,bufWidth*bufHeight*4);
		for(int i=0;i<bufHeight;i++)
			for(int j=0;j<bufWidth;j++){
				zBuffer[i*bufWidth + j]=2.0f;
			}
	}
	int bufWidth;
	int bufHeight;
	unsigned char *pBuffer;
	float *zBuffer;
	Device *device;
};

void SetMatrixTranslate(Mat &m,float x,float y,float z){
	m[0][0]=m[1][1]=m[2][2]=m[3][3]=1.0f;
	m[3][0]=x;m[3][1]=y;m[3][2]=z;
}

// in radian
void SetMatrixRotate(Mat &m,float x, float y,float z,float theta){
	float nor = 1/sqrt(x*x+y*y+z*z);
	float cos2 = cos(theta * 0.5f);
	float sin2 = sin(theta * 0.5f);
	float X = x * sin2 * nor;
	float Y = y * sin2 * nor;
	float Z = z * sin2 * nor;
	float W = cos2;
	m[0][0]=1-2*Y*Y-2*Z*Z;	m[1][0]=2*X*Y-2*Z*W;	m[2][0]=2*X*Z+2*Y*W;	m[3][0]=0;
	m[0][1]=2*X*Y+2*Z*W;	m[1][1]=1-2*X*X-2*Z*Z;	m[2][1]=2*Y*Z-2*X*W;	m[3][1]=0;
	m[0][2]=2*X*Z-2*Y*W;	m[1][2]=2*Y*Z+2*X*W;	m[2][2]=1-2*X*X-2*Y*Y;	m[3][2]=0;
	m[0][3]=0;				m[1][3]=0;				m[2][3]=0;				m[3][3]=1;
}

void SetMatrixPerspective(Mat &m,float fov,float aspect,float n,float f){
	float fax = 1.0f/tan( fov * 0.5);
	m[0][0] = fax / aspect;
	m[1][1] = fax;
	m[2][2] = (f+n)/(f-n);
	m[3][2] = -2*n*f/(f-n);
	m[2][3] = 1;
}

void Vec4MultMatrix(Vec4 &a,Vec4 &b,Mat &m){
	a.x = m[0][0] * b.x + m[1][0] * b.y + m[2][0] * b.z + m[3][0] * b.w;
	a.y = m[0][1] * b.x + m[1][1] * b.y + m[2][1] * b.z + m[3][1] * b.w;
	a.z = m[0][2] * b.x + m[1][2] * b.y + m[2][2] * b.z + m[3][2] * b.w;
	a.w = m[0][3] * b.x + m[1][3] * b.y + m[2][3] * b.z + m[3][3] * b.w;
}


void Vec4Normalize(Vec4 &a){
	float invw = 1.0f;
	if(a.w < -1e-8 || a.w > 1e-8)invw = 1.0f / a.w;
	a.x *= invw;
	a.y *= invw;
	a.z *= invw;
	a.w = 1.0f;
}

void MatrixMultMatrix(Mat &des,Mat &a,Mat b){
	for(int i=0;i<4;i++)
		for(int j=0;j<4;j++){
			des[i][j]=a[i][0]*b[0][j]+a[i][1]*b[1][j]+a[i][2]*b[2][j]+a[i][3]*b[3][j];
		}
}

int main(){
	Device *device = new Device();
	Renderer *renderer = new Renderer(device);
	Vec4 a(-1.0f, -1.0f, -1.0f);
	Vec4 b( 1.0f, -1.0f, -1.0f);
	Vec4 c(-1.0f,  1.0f, -1.0f);
	Vec4 d( 1.0f,  1.0f, -1.0f);
	Vec4 e(-1.0f, -1.0f,  1.0f);
	Vec4 f( 1.0f, -1.0f,  1.0f);
	Vec4 g(-1.0f,  1.0f,  1.0f);
	Vec4 h( 1.0f,  1.0f,  1.0f);

	Mesh cube;
	cube.AddTriangle(a, b, c);
	cube.AddTriangle(b, d, c);

	cube.AddTriangle(e, a, g);
	cube.AddTriangle(a, c, g);

	cube.AddTriangle(f, e, h);
	cube.AddTriangle(e, g, h);

	cube.AddTriangle(b, f, d);
	cube.AddTriangle(f, h, d);

	cube.AddTriangle(c, d, g);
	cube.AddTriangle(d, h, g);

	cube.AddTriangle(e, f, a);
	cube.AddTriangle(f, b, a);


	MSG msg;
	float thetaPer=PI*0.2f;
	float theta=0.0f;
	float lastTime=0.0f;
	float curTime=0.0f;
	bool init=true;
	while(TRUE){
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
			if(msg.message == WM_QUIT)break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			curTime=GetTickCount();
			if(init){
				lastTime=curTime;
				init=false;
			}
			if(curTime-lastTime<20)continue;
			float fac=(curTime-lastTime)*0.001;
			theta=theta+fac*thetaPer;
			lastTime=curTime;
			// transform----------------
			Mat T,R;
			SetMatrixRotate(R,0.0f,1.0f,0.0f,theta);
			SetMatrixTranslate(T,0.0f,0.0f,5.0f);
			Mat des;
			des = R*T;
			Mat p;
			SetMatrixPerspective(p,PI*0.5f,(float)renderer->bufWidth/renderer->bufHeight,1.0f,500.0f);
			des=des*p;
			//-------------------------
			renderer->Clear();
			renderer->DrawMesh(cube,des);
			renderer->Update();
		}
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch(uMsg){
		case WM_CREATE:
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}
