#include "Render.h"


#include <sstream>

#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"
#include <iostream>
#include <vector>
#include <random>

GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;


//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Shader s[10];  //массивчик для десяти шейдеров

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 25;
		fi1 = 1;
		fi2 = 30/57.;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	virtual void SetUpCamera()
	{
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	virtual void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}

	virtual Vector3 Getforward() {
		return (lookPoint - pos).normolize();
	}

}CAM  ;   

//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);
		
		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окружность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света

int mapWH = 10;

struct lowModel {
	int x, y, angl;
	lowModel(int x, int y, int angl): x(x), y(y), angl(angl){}
};

class hitBox {
public:
	double width, height, depth;
	double posX, posY, posZ, angl;

	hitBox(double width, double height, double depth, double posX, double posY, double posZ)
		: width(width), height(height), depth(depth), posX(posX), posY(posY), posZ(posZ), angl(0) {}

	void getHitBoxParam(double& width, double& height, double& depth) {
		width = this->width;
		height = this->height;
		depth = this->depth;
	}

	bool isColliding(hitBox other[], int lenth, bool onlyFront = 1, bool isVisible = 1) {
		if (isVisible) {
			double thisFrontX = posX + height / 2.0 * cos(angl / 57. - PI / 2.),
				thisFrontY = posZ + height / 2.0 * sin(angl / 57. - PI / 2.),
				thisBackX = posX - height / 2.0 * cos(angl / 57. - PI / 2.),
				thisBackY = posZ - height / 2.0 * sin(angl / 57. - PI / 2.),
				thisLeftX = posX + width / 2.0 * cos(angl / 57.),
				thisLeftY = posZ + width / 2.0 * sin(angl / 57.),
				thisRightX = posX - width / 2.0 * cos(angl / 57.),
				thisRightY = posZ - width / 2.0 * sin(angl / 57.);

			double otherMinX, otherMaxX, otherMinY, otherMaxY;

			for (int i{ 0 }; i <= lenth; i++) {
				otherMinX = other[i].posX - other[i].width / 2.;
				otherMaxX = other[i].posX + other[i].width / 2.;
				otherMinY = other[i].posZ - other[i].height / 2.;
				otherMaxY = other[i].posZ + other[i].height / 2.;

				//отображение точек хитбокса
				/*glPointSize(5.0);
				glColor3f(1.0, 0.0, 0.0);
				glBegin(GL_POINTS);
				glVertex3d(thisFrontX, thisFrontY, 1);
				glVertex3d(thisBackX, thisBackY, 1);
				glVertex3d(thisLeftX, thisLeftY, 1);
				glVertex3d(thisRightX, thisRightY, 1);
				glEnd();*/

				if (!onlyFront) {
					// Проверка по оси X
					if (thisFrontX >= otherMinX && thisFrontX <= otherMaxX
						|| thisBackX >= otherMinX && thisBackX <= otherMaxX
						|| thisLeftX >= otherMinX && thisLeftX <= otherMaxX
						|| thisRightX >= otherMinX && thisRightX <= otherMaxX)
						// Проверка по оси Y
						if (thisFrontY >= otherMinY && thisFrontY <= otherMaxY
							|| thisBackY >= otherMinY && thisBackY <= otherMaxY
							|| thisLeftY >= otherMinY && thisLeftY <= otherMaxY
							|| thisRightY >= otherMinY && thisRightY <= otherMaxY)
							return true;
				}
				else if (thisFrontX >= otherMinX && thisFrontX <= otherMaxX)
					if (thisFrontY >= otherMinY && thisFrontY <= otherMaxY)
						return true;
			}

			return false;
		}
	}
};

class item : public hitBox {
public:
	bool visible{1};
	double points;

	item(double width, double height, double depth, double posX, double posY, double posZ)
		: visible(1), points(1), hitBox(width, height, depth, posX, posY, posZ) {}
};

class character : public hitBox {
public:
	double velocity, points, lastHitTime{ 0 };
	bool frontBlock{ 0 }, backBlock{ 0 };
	bool visible{ 1 };
	int dieAngl{ 0 }, HP{1};

	character(double width, double height, double depth, double posX, double posY, double posZ)
		: velocity(0), hitBox(width, height, depth, posX, posY, posZ){}

	int isCollidingVEC(std::vector<item> other, bool onlyFront = 0) {
		double thisFrontX = posX + height / 2.0 * cos(angl / 57. - PI / 2.),
			thisFrontY = posZ + height / 2.0 * sin(angl / 57. - PI / 2.),
			thisBackX = posX - height / 2.0 * cos(angl / 57. - PI / 2.),
			thisBackY = posZ - height / 2.0 * sin(angl / 57. - PI / 2.),
			thisLeftX = posX + width / 2.0 * cos(angl / 57.),
			thisLeftY = posZ + width / 2.0 * sin(angl / 57.),
			thisRightX = posX - width / 2.0 * cos(angl / 57.),
			thisRightY = posZ - width / 2.0 * sin(angl / 57.);

		double otherMinX{ 0 }, otherMaxX{ 0 }, otherMinY{ 0 }, otherMaxY{ 0 };

		for (int i{ 0 }; i < other.size(); i++) {
			otherMinX = other[i].posX - other[i].width / 2.;
			otherMaxX = other[i].posX + other[i].width / 2.;
			otherMinY = other[i].posZ - other[i].height / 2.;
			otherMaxY = other[i].posZ + other[i].height / 2.;

			if (!onlyFront) {
				// Проверка по оси X
				if (thisFrontX >= otherMinX && thisFrontX <= otherMaxX
					|| thisBackX >= otherMinX && thisBackX <= otherMaxX
					|| thisLeftX >= otherMinX && thisLeftX <= otherMaxX
					|| thisRightX >= otherMinX && thisRightX <= otherMaxX)
					// Проверка по оси Y
					if (thisFrontY >= otherMinY && thisFrontY <= otherMaxY
						|| thisBackY >= otherMinY && thisBackY <= otherMaxY
						|| thisLeftY >= otherMinY && thisLeftY <= otherMaxY
						|| thisRightY >= otherMinY && thisRightY <= otherMaxY)
						return i+1;
			}
			else if (thisFrontX >= otherMinX && thisFrontX <= otherMaxX)
				if (thisFrontY >= otherMinY && thisFrontY <= otherMaxY)
					return i+1;
		}

		return 0;
	}

	void alive() {
		posX = 0;
		posZ = 0;
		visible = true;
		dieAngl = 0;
		frontBlock = false;
		backBlock = false;
		points = 0;
		HP = 1;
		lastHitTime = 0;
	}
};

class fromTo : public hitBox {
private: 
	int endPosX, endPosY, startPosX, startPosY;
public:
	bool visible{ 0 };
	double velocity{0.2};

	fromTo(double width, double height, double depth)
		: hitBox(width, height, depth, -10, 0, -10) {}

	void move() {
		if (fabs(posX) >= mapWH || fabs(posZ) >= mapWH) {
			std::random_device rd;
			std::mt19937 gen(rd());

			std::uniform_int_distribution<int> distribution(-mapWH, mapWH);

			startPosX = distribution(gen);
			if (startPosX > 0) {
				startPosX = mapWH;
				startPosY = distribution(gen);
				endPosX = -mapWH;
				endPosY = distribution(gen);
			}else {
				startPosY = -mapWH;
				startPosX = distribution(gen);
				endPosY = mapWH;
				endPosX = distribution(gen);
			}
			angl = fabs(atan2(endPosX - startPosX, endPosY - startPosY)) * 57;
			posX = startPosX;
			posZ = startPosY;
		}

		int deltaX = endPosX - startPosX;
		int deltaY = endPosY - startPosY;

		double distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

		double stepX = velocity * deltaX / distance;
		double stepZ = velocity * deltaY / distance;

		posX += stepX;
		posZ += stepZ;

		if (fabs(posX) >= mapWH || fabs(posZ) >= mapWH)
			visible = 0;
	}
};

character Fox(0.5, 2.5, 2, 0, 0, 0);

class WASDcamera :public CustomCamera
{
public:
	WASDcamera()
	{
		lookPoint.setCoords(0, 0, 2);
		pos.setCoords(Fox.posX, Fox.posZ, 2);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{
		pos.setCoords(cos(fi1) + Fox.posX,
			sin(fi1) + Fox.posZ,
			sin(fi2)+2);
		lookPoint.setCoords(Fox.posX, Fox.posZ, 2);
		LookAt();
	}
} WASDCAM;

//старые координаты мыши
int mouseX = 0, mouseY = 0;

int centerX, centerY;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;
bool stop{ 0 }, isHitBox{ 0 }, pause{0};

Camera* camera = &CAM;
int cameraType = 0;

void centerMouse(HWND hwnd) {//УРАА
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	centerX = clientRect.left + (clientRect.right - clientRect.left) / 2;
	centerY = clientRect.top + (clientRect.bottom - clientRect.top) / 2;

	// Преобразуем координаты в экранные
	POINT clientToScreenPoint;
	clientToScreenPoint.x = centerX;
	clientToScreenPoint.y = centerY;
	ClientToScreen(hwnd, &clientToScreenPoint);

	// Устанавливаем указатель мыши в центр окна
	SetCursorPos(clientToScreenPoint.x, clientToScreenPoint.y);
}

//обработчик движения мыши
void mouseEvent(OpenGL* ogl, int mX, int mY){
	double dx = centerX - mX;
	double dy = centerY - mY;

	if (dx == 0 && dy == 0) { return; }

	if (cameraType == 1)
	{
		centerMouse(ogl->getHwnd());

		double sensitivity = 0.01;
		dx *= sensitivity;
		dy *= -sensitivity;

		camera->fi1 += dx;
		camera->fi2 += dy;
	}

	if (OpenGL::isKeyPressed(VK_RBUTTON)){
		dx = mouseX - mX;
		dy = mouseY - mY;

		camera->fi1 += 0.01 * dx;
		camera->fi2 += -0.01 * dy;
	}
	mouseX = mX;
	mouseY = mY;
}


//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL *ogl, int delta)
{

	float _tmpZ = delta*0.003;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera->camDist <= 1)
		return;
	if (delta > 0 && camera->camDist >= 100)
		return;
	
	camera->camDist += 0.01*delta;
}

ObjFile MFox, MApple, MTrash, MTree, MRock, MGrass, MFlower, MLowgrass, MKaban;

Texture TFox, TApple, TTrash, TTree, TRock, TGrass, TFlower, TLowgrass, TKaban, THP, TGapple, TFon;

const double scaleFox = 0.02; //коэф уменьшения модельки

std::vector<lowModel> Flower, Lowgrass;

std::vector<item> Apple;
const int appleCount = 5;
const int updateApple = 10;
int updateTimeApple = updateApple;

item Gapple(0.8, 0.8, 2, -8, 0, -6);
const int updateGapple = 5;
int updateTimeGapple = updateGapple;

std::vector<item> Trash;
const int trashCount = 15;

hitBox Tree[3] = { hitBox(0.8, 0.8, 2, -8, 0, -6),
				hitBox(0.8, 0.8, 2, 4, 0, 2) ,
				hitBox(0.8, 0.8, 2, -3, 0, 8) };

hitBox Rock(8, 8, 8, 6, 2, -6);
fromTo Kaban(2, 2, 2);

int record{ 0 }, dieCount{ 0 };

//обработчик нажатия кнопок клавиатуры
void keyDownEvent(OpenGL* ogl, int key) {
	//респавн
	if (OpenGL::isKeyPressed('R')) {
		Fox.alive();
		updateTimeApple = updateApple;
		updateTimeGapple = updateGapple;
		Time = 0;
		stop = false;
	}

	//хитбоксы
	if (OpenGL::isKeyPressed('B'))
		isHitBox = !isHitBox;

	//пауза
	if (OpenGL::isKeyPressed('P'))
		pause = !pause;

	//первое лиц
	if (OpenGL::isKeyPressed('F')) {
		if (cameraType == 1) {
			cameraType = 0;
			ogl->mainCamera = &CAM;
			camera = &CAM;
		}
		else if (cameraType == 0) {
			cameraType = 1;
			ogl->mainCamera = &WASDCAM;
			camera = &WASDCAM;

			centerMouse(ogl->getHwnd());
		}
	}
}

void keyUpEvent(OpenGL* ogl, int key) {}


void getRandXY(int&, int&);
void getRandAngl(int&);

void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	ogl->mainCamera = camera;

	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
	

	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем

	

	 //так как гит игнорит модели *.obj файлы, так как они совпадают по расширению с объектными файлами, 
	 // создающимися во время компиляции, я переименовал модели в *.obj_m

#define modelTexture(name,obj,bmp) loadModel("models\\"##obj".object", &M##name);\
	T##name.loadTextureFromFile("textures//"##bmp".bmp");\
	T##name.bindTexture();

	glActiveTexture(GL_TEXTURE0);

	modelTexture(Fox, "Lowpoly_Fox", "Lowpoly_Fox");

	modelTexture(Apple, "apple", "Rapple");

	modelTexture(Trash, "carrot", "carrot");

	modelTexture(Tree, "tree", "tree");

	modelTexture(Rock, "Rock", "Rock");

	modelTexture(Grass, "Cube", "grass");

	modelTexture(Flower, "flower", "flower");

	modelTexture(Lowgrass, "lowgrass", "lowgrass2");

	modelTexture(Kaban, "goose", "goose");
	
	TGapple.loadTextureFromFile("textures//Gapple.bmp");

	THP.loadTextureFromFile("textures//heart.bmp");

	TFon.loadTextureFromFile("textures//fon1.bmp");

	tick_n = GetTickCount();
	tick_o = tick_n;

	int x, y, angl;
	for (int i{ 1 }; i <= appleCount; i++) {
		getRandXY(x, y);
		Apple.push_back(item(0.5, 0.5, 1, x, 0, y));
	}
	for (int i{ 1 }; i <= trashCount; i++) {
		do// чтоб морковки не спавнились в центре
			getRandXY(x, y);
		while (x >= -1 && x <= 1 && y <= 1 && y >= -1);
		Trash.push_back(item(0.8, 0.8, 1, x, 0, y));
	}

	for (int i{ 1 }; i <= 30; i++) {
		getRandXY(x, y);
		getRandAngl(angl);
		Flower.push_back(lowModel(x, y, angl));
	}

	for (int i{ 1 }; i <= 60; i++) {
		getRandXY(x, y);
		getRandAngl(angl);
		Lowgrass.push_back(lowModel(x, y, angl));
	}

	Fox.velocity = 4 * scaleFox;
	Kaban.velocity = 0.15;
	Gapple.visible = false;
}

void normalize(double& x, double& y) {
	double lenth = sqrt(x*x + y*y);
	x /= lenth;
	y /= lenth;
}

void RenderHitBox(std::vector<item> boxes, int ZY = 0)
{
	double width, depth, height;
	for (auto& box : boxes) {
		box.getHitBoxParam(width, depth, height);

		PUSH;
			glTranslated(box.posX, ZY ? box.posZ : box.posY, ZY ? box.posY : box.posZ);
			glRotated(box.angl, 0.0, 0.0, 1.0);
			glBegin(GL_QUADS);
				glVertex3d(-width / 2.0, -depth / 2.0, 1.0);
				glVertex3d(width / 2.0, -depth / 2.0, 1.0);
				glVertex3d(width / 2.0, depth / 2.0, 1.0);
				glVertex3d(-width / 2.0, depth / 2.0, 1.0);
			glEnd();
		POP;
	}
}

void RenderHitBox(hitBox& box, int ZY = 0)
{
	double width, depth, height;
	box.getHitBoxParam(width, depth, height);

	PUSH;
	glTranslated(box.posX, ZY ? box.posZ : box.posY, ZY ? box.posY : box.posZ);
	glRotated(box.angl, 0.0, 0.0, 1.0);
	glBegin(GL_QUADS);
	glVertex3d(-width / 2.0, -depth / 2.0, 1.0);
	glVertex3d(width / 2.0, -depth / 2.0, 1.0);
	glVertex3d(width / 2.0, depth / 2.0, 1.0);
	glVertex3d(-width / 2.0, depth / 2.0, 1.0);
	glEnd();
	POP;
}


void move() {
	double x = camera->Getforward().X(),
		y = camera->Getforward().Y();

	normalize(x, y); //делаем единичный вектор

	if (!Fox.dieAngl) {
		if (!Fox.frontBlock)
			if (OpenGL::isKeyPressed('W')) {
				if ((Fox.isColliding(Tree, 2, 1) || Fox.isColliding(&Rock, 0,1)) && !(Fox.frontBlock || Fox.backBlock)) {
					Fox.frontBlock = 1;
					Fox.backBlock = 0;
				}
				else {
					Fox.frontBlock = 0;
					Fox.backBlock = 0;
				}
				Fox.posZ += y * Fox.velocity;
				Fox.posX += x * Fox.velocity;
				Fox.angl = camera->fi1 * 57 - 90;
			}

		if (!Fox.backBlock)
			if (OpenGL::isKeyPressed('S')) {
				if ((Fox.isColliding(Tree, 2,1) || Fox.isColliding(&Rock, 0,1)) && !(Fox.frontBlock || Fox.backBlock)) {
					Fox.frontBlock = 0;
					Fox.backBlock = 1;
				}
				else {
					Fox.frontBlock = 0;
					Fox.backBlock = 0;
				}
				Fox.posZ -= y * Fox.velocity;
				Fox.posX -= x * Fox.velocity;
				Fox.angl = camera->fi1 * 57 + 90;
			}
	}

	if (Fox.isCollidingVEC(Trash, 1)
		|| fabs(Fox.posX) > mapWH
		|| fabs(Fox.posZ) > mapWH
		|| Fox.dieAngl > 0)
		if (Time - Fox.lastHitTime >= 1) {
			--Fox.HP;
			Fox.lastHitTime = Time;
		}

	if (Fox.isColliding(&Kaban, 0, 0, Kaban.visible))		
		if (Time - Fox.lastHitTime >= 1) {
			Fox.HP -= 2;
			Fox.lastHitTime = Time;
		}

	if (Fox.HP <= 0)
		if (Fox.dieAngl != 45) {
			Fox.dieAngl++;
			Fox.dieAngl == 1 ? dieCount++: dieCount;
			stop = true;
			record = Fox.points > record ? Fox.points : record;
		}
		else
			Fox.visible = 0;

	if (Fox.isColliding(&Gapple, 0, 0, Gapple.visible)) {
		Gapple.visible = false;
		Fox.HP++;
	}

	int appleInd = Fox.isCollidingVEC(Apple);
	if (appleInd--) {
		Fox.points += Apple[appleInd].points;
		Apple.erase(Apple.begin() + appleInd);
	}
}

void getRandXY(int& x, int& y) {
	std::random_device rd;
	std::mt19937 gen(rd());

	int mapSizeStart = -mapWH;
	int mapSizeEnd = mapWH;

	std::uniform_int_distribution<int> distribution(mapSizeStart, mapSizeEnd);

	x = distribution(gen);
	y = distribution(gen);
}

void getRandAngl(int& angl) {
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> distribution(-180, 180);

	angl = distribution(gen);
}

void kabanGo() {
	if(Kaban.visible)
		Kaban.move();
}

void Render(OpenGL* ogl)
{
	if(!pause)
		if (!stop) {
			tick_o = tick_n;
			tick_n = GetTickCount();
			Time += (tick_n - tick_o) / 1000.0;
		}

	if (Time >= updateTimeGapple && Gapple.visible == false) {
		int x, y;
		getRandXY(x, y);
		Gapple.posX = x;
		Gapple.posZ = y;
		Gapple.visible = true;
		updateTimeGapple += updateGapple;
	}

	if (Time >= updateTimeGapple && Gapple.visible == true) {
		Gapple.visible = false;
		updateTimeGapple += updateGapple;
	}

	if (Time >= updateTimeApple) { //проверка времени респавна яблок
		int x, y;
		Apple.clear();
		for (int i{ 1 }; i <= appleCount; i++) {
			getRandXY(x, y);
			Apple.push_back(item(1, 1, 1, x, 0, y));
		}
		updateTimeApple += updateApple;
	}

	if (1)
		Kaban.visible = true;

	glEnable(GL_DEPTH_TEST);

	if (!pause) {
		kabanGo();
		move();
	}

	//хитбоксы
	if (isHitBox) {
		RenderHitBox(Fox, 1);
		RenderHitBox(Apple, 1);
		RenderHitBox(Trash, 1);
		RenderHitBox(Tree[0], 1);
		RenderHitBox(Tree[1], 1);
		RenderHitBox(Tree[2], 1);
		RenderHitBox(Rock, 1);
		RenderHitBox(Kaban, 1);
	}

	//цветы
	s[1].UseShader();
	for (auto& flower : Flower) {
		PUSH;
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslated(flower.x, 0, -flower.y);
		glRotated(flower.angl, 0.0, 1.0, 0.0);
		glScaled(2, 2, 2);
		TFlower.bindTexture();
		MFlower.DrawObj();
		POP;
	}

	//низкая трава
	for (auto& grass : Lowgrass) {
		PUSH;
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslated(grass.x, 0, -grass.y);
		glRotated(grass.angl, 0.0, 1.0, 0.0);
		glScaled(0.01, 0.01, 0.01);
		TLowgrass.bindTexture();
		MLowgrass.DrawObj();
		POP;
	}

	//трава
	PUSH;
	glScaled(40, 40, 3);
	glTranslated(0, 0, -0.25);
	TGrass.bindTexture();
	MGrass.DrawObj();
	POP;

	//Скала
	PUSH;
	glRotated(90, 1.0, 0.0, 0.0);
	glTranslated(Rock.posX, Rock.posY, -Rock.posZ);
	glScaled(5, 5, 5);
	TRock.bindTexture();
	MRock.DrawObj();
	POP;

	//деревья
	Tree[0].angl = 180;
	Tree[1].angl = -76;
	Tree[2].angl = 18;
	for (auto& tree : Tree) {
		PUSH;
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslated(tree.posX, tree.posY, -tree.posZ);
		glRotated(tree.angl, 0.0, 1.0, 0.0);
		glScaled(0.01, 0.01, 0.01);
		TTree.bindTexture();
		MTree.DrawObj();
		POP;
	}

	//морковь
	for (auto& Trash : Trash) {
		PUSH;
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslated(Trash.posX, Trash.posY, -Trash.posZ);
		glScaled(0.1, 0.1, 0.1);
		TTrash.bindTexture();
		MTrash.DrawObj();
		POP;
	}

	//яблоко
	for (auto& Apple : Apple) {
		PUSH;
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslated(Apple.posX, Apple.posY, -Apple.posZ);
		glScaled(0.1, 0.1, 0.1);
		TApple.bindTexture();
		MApple.DrawObj();
		POP;
	}

	//хилл яблоко
	if (Gapple.visible) {
		PUSH;
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslated(Gapple.posX, Gapple.posY, -Gapple.posZ);
		glScaled(0.1, 0.1, 0.1);
		TGapple.bindTexture();
		MApple.DrawObj();
		POP;
	}

	//Гусь
	if (Kaban.visible) {
		PUSH;
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslated(Kaban.posX, Kaban.posY, -Kaban.posZ);
		glScaled(1, 1, 1);
		glRotated(Kaban.angl, 0.0, 1.0, 0.0);
		TKaban.bindTexture();
		MKaban.DrawObj();
		POP;
	}

	//лиса
	if (Fox.visible) {
		PUSH;
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslated(Fox.posX, Fox.posY, -Fox.posZ);
		glScaled(scaleFox, scaleFox, scaleFox);
		glRotated(Fox.angl, 0.0, 1.0, 0.0);
		glRotated(Fox.dieAngl, 0.0, 0.0, 1.0);
		TFox.bindTexture();
		MFox.DrawObj();
		POP;
	}

	//фон
	if (cameraType == 1) {
		glEnable(GL_TEXTURE_2D);
		TFon.bindTexture();
		PUSH;
		glTranslated(0, 0, -5);
		glBegin(GL_QUAD_STRIP);
		for (int i = 0; i <= 10; ++i) {
			float a = (2. * M_PI * i) / 10;
			float x = 20 * cos(a);
			float y = 20 * sin(a);

			glTexCoord2f(i / 10., 0.);
			glVertex3f(x, y, 0);

			glTexCoord2f(i / 10., 1.);
			glVertex3f(x, y, 20);
		}
		glEnd();
		POP;
	}
	Shader::DontUseShaders();
}


bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL* ogl)
{
	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);


	glActiveTexture(GL_TEXTURE0);

	GuiTextRectangle rec;
	rec.setSize(500, 400);
	rec.setPosition(10, ogl->getHeight() - 400 - 10);


	std::stringstream ss;
	ss << "Время: " << Time << std::endl;
	ss << "Сменить вид - F" << std::endl;
	ss << "Пауза - P" << std::endl;
	ss << "Переродиться - R" << std::endl;
	ss << "Включить хитбоксы - B" << std::endl << std::endl;
	ss << "Коорд. лисы: (" << Fox.posX << ", " << Fox.posZ << ", " << Fox.posY << ")" << " angl = " << (int)Fox.angl % 360 << std::endl;
	ss << "Очки: " << Fox.points << std::endl;
	ss << "~~~~~~~~~~~~~" << std::endl;
	ss << "Рекорд: " << record << std::endl;
	ss << "Число смертей: " << dieCount << std::endl;
	rec.setText(ss.str().c_str());
	rec.Draw();

	glEnable(GL_TEXTURE_2D);
	THP.bindTexture();

	double center = ogl->getWidth() / 2.;
	
	for (int i{ 1 }; i <= Fox.HP; i++) {
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(center - 150/2. + 50 * (i-1), 50);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(center - 150 / 2. + 50 * (i), 50);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(center - 150 / 2. + 50 * (i), 50 + 50);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(center - 150 / 2. + 50 * (i - 1), 50 + 50);
		glEnd();
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	if (stop) {
		GuiTextRectangle die;
		die.setSize(800, 400);
		die.setPosition(ogl->getWidth()/2. - 400, ogl->getHeight()/2.- 200);

		glColor4f(1,0,0,0.2);
		glBegin(GL_QUADS);
		glVertex2f(0,0);
		glVertex2f(ogl->getWidth(), 0);
		glVertex2f(ogl->getWidth(), ogl->getHeight());
		glVertex2f(0, ogl->getHeight());
		glEnd();

		std::string str = "УМЕР";
		die.setText(str.c_str(),300,0,0,255);
		die.Draw();
	}

	GuiTextRectangle rec_;
	rec_.setSize(100, 100);
	rec_.setPosition(ogl->getWidth() - 100, ogl->getHeight() - 100);
	std::stringstream coord;

	coord << mouseX << " " << mouseY << std::endl;
	coord << ogl->getWidth() << " " << ogl->getHeight() << std::endl;
	coord << centerX << " " << centerY << std::endl;

	rec_.setText(coord.str().c_str());
	rec_.Draw();

	Shader::DontUseShaders();

}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

