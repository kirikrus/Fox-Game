#include "MyOGL.h"

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>


#include "Camera.h"
#include "Light.h"
#include "PrimitivesStatic.h"


#define DRAW_TO_WINDOW


#ifndef DRAW_TO_WINDOW

void OpenGL::createBitmap()
{

	tagRECT wndrect;
	GetClientRect(g_hWnd, &wndrect);

	int w = wndrect.right;// -wndrect.right;
	int h = wndrect.bottom;// -wndrect.top;
	char *bits;
	//bits = (char *)malloc(w*h * 3 * sizeof(char));

	bitmap_hdc = CreateCompatibleDC(0);

	BITMAPINFOHEADER binfo;
	memset(&binfo, 0, sizeof(BITMAPINFOHEADER));

	binfo.biBitCount = 24;
	binfo.biWidth = w;
	binfo.biHeight = h;
	binfo.biSize = sizeof(binfo);
	binfo.biPlanes = 1;
	binfo.biCompression = BI_RGB;

	bitmap = CreateDIBSection(bitmap_hdc, (BITMAPINFO*)&binfo, DIB_RGB_COLORS, 0, 0, 0);
	//bitmap = CreateCompatibleBitmap(bitmap_hdc, w, h);
	SelectObject(bitmap_hdc, bitmap);
	

}

#endif

OpenGL::OpenGL()
{
	
}
OpenGL::~OpenGL()
{

}

void OpenGL::setHWND(HWND window)
{
	g_hWnd = window;
	tagRECT r;
	GetClientRect(g_hWnd, &r);
	width = r.right;
	height = r.bottom;
}


void OpenGL::mouseMovie(int mX, int mY)
{
	for (unsigned char i = 0; i < mouseFunc.size(); i++)
	{
		(*mouseFunc[i])(this, mX, mY);
	}
	OldMouseX = mX;
	OldMouseY = mY;
}

void OpenGL::wheelEvent(float delta)
{
	for (unsigned char i = 0; i < wheelFunc.size(); i++)
	{
		(*wheelFunc[i])(this, delta);
	}

}

void OpenGL::keyDownEvent(int key)
{
	for (unsigned char i = 0; i < keyDownFunc.size(); i++)
	{
		(*keyDownFunc[i])(this, key);
	}

}


void OpenGL::keyUpEvent(int key)
{
	for (unsigned char i = 0; i < keyUpFunc.size(); i++)
	{
		(*keyUpFunc[i])(this, key);
	}

}


void OpenGL::DrawAxes()
{
	bool f1 = glIsEnabled(GL_LIGHTING);
	glDisable(GL_LIGHTING);
	bool f2 = glIsEnabled(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_2D);
	

	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(10, 0, 0);

	glColor3f(0, 1, 0);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 10, 0);

	glColor3f(0, 0, 1);
	glVertex3d(0, 0, 0);
	glVertex3d(0, 0, 10);
	glEnd();

	glColor3f(0.0f, 0.0f, 0.0f);

	if (f1)
	glEnable(GL_LIGHTING);
	if (f2)
	glEnable(GL_TEXTURE_2D);
}



void OpenGL::render()
{
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, aspect, .1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	
	mainCamera->SetUpCamera();
	mainLight->SetUpLight();
	
	glEnable(GL_DEPTH_TEST);
	//DrawAxes();
	//drawPlane();
	
	
	for (unsigned char i = 0; i < renderFunc.size(); i++)
	{
		(*(renderFunc[i]))(this);
	}
	
	mainLight->DrawLightGhismo();


	if (renderGuiFunc != nullptr)
		(*renderGuiFunc)(this);


	glDisable(GL_LIGHTING);

	//drawPlane();
#ifdef DRAW_TO_WINDOW
	SwapBuffers(g_hDC);
#endif




	//рисуем сообщение вверху слева:
	
	//прямоугольник для текста
	tagRECT r;
	r.left = 10;
	r.top = 10;
	r.right = width;
	r.bottom = 200;

	//рисуем текст
	DrawText(g_hDC, message.c_str(), -1, &r, 0);

#ifndef DRAW_TO_WINDOW	
	BitBlt(dc, 0, 0, width, height, bitmap_hdc, 0, 0, SRCCOPY);
#endif
}

void OpenGL::resize(int w, int h)
{
	width = w;
	height = h;

#ifndef DRAW_TO_WINDOW
	BITMAPINFOHEADER binfo;
	memset(&binfo, 0, sizeof(BITMAPINFOHEADER));

	binfo.biBitCount = 24;
	binfo.biWidth = w;
	binfo.biHeight = h;
	binfo.biSize = sizeof(binfo);
	binfo.biPlanes = 1;
	binfo.biCompression = BI_RGB;

	DeleteObject(bitmap);
	//bitmap = CreateDIBSection(bitmap_hdc, (BITMAPINFO*)&binfo, DIB_RGB_COLORS, 0, 0, 0);
	bitmap = CreateCompatibleBitmap(bitmap_hdc, w, h);


	SelectObject(bitmap_hdc, bitmap);
#endif

	glViewport(0, 0, width, height);


	aspect = (GLdouble)width / (GLdouble)height;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (resizeFunc != nullptr)
		resizeFunc(this, w, h);
}



void OpenGL::init(void)
{
#ifndef DRAW_TO_WINDOW
	createBitmap();
	g_hDC = bitmap_hdc;
	dc = GetDC(g_hWnd);
#else
	g_hDC = GetDC(g_hWnd);
#endif


	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
#ifndef DRAW_TO_WINDOW
	pfd.dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI;
#else
	pfd.dwFlags = PFD_DRAW_TO_WINDOW  | PFD_SUPPORT_OPENGL |  PFD_DOUBLEBUFFER;
#endif
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;

	
	GLuint iPixelFormat = ChoosePixelFormat(g_hDC, &pfd);

	if (iPixelFormat != 0)
	{
		PIXELFORMATDESCRIPTOR bestMatch_pfd;
		DescribePixelFormat(g_hDC, iPixelFormat, sizeof(pfd), &bestMatch_pfd);

		if (bestMatch_pfd.cDepthBits < pfd.cDepthBits)
		{
			return;
		}

		if (SetPixelFormat(g_hDC, iPixelFormat, &pfd) == FALSE)
		{
			DWORD dwErrorCode = GetLastError();
			return;
		}
	}
	else
	{
		DWORD dwErrorCode = GetLastError();
		return;
	}

	g_hRC = wglCreateContext(g_hDC);
	wglMakeCurrent(g_hDC, g_hRC);
	glClearColor(107/255., 145 / 255., 179 / 255., 1.0f);
	glEnable(GL_DEPTH_TEST);


	//выполняем пользовательские функции инфициализации
	for (unsigned char i = 0; i < initFunc.size(); i++)
	{
		(*initFunc[i])(this);
	}

	SphereStatic::MakeVertex();
	CircleStatic::MakeVertex();

}