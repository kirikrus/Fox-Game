#include "Render.h"


#include <sstream>

#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
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
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
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

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}

	auto Getforward() {
		return (lookPoint - pos).normolize();
	}

}  camera;   //создаем объект камеры


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

	bool isColliding(hitBox other[], int lenth) {
		double thisFrontX = posX + height / 2.0 * cos(angl / 57. - PI / 2.),
			thisFrontY = posZ + height / 2.0 * sin(angl / 57. - PI / 2.);

		double otherMinX, otherMaxX, otherMinZ, otherMaxZ;

		for (int i{ 0 }; i <= lenth; i++) {
			otherMinX = other[i].posX - other[i].width / 2.;
			otherMaxX = other[i].posX + other[i].width / 2.;
			otherMinZ = other[i].posZ - other[i].height / 2.;
			otherMaxZ = other[i].posZ + other[i].height / 2.;

			//отображение точек хитбокса
			/*glPointSize(5.0);
			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_POINTS);
				glVertex3d(thisFrontX, thisFrontY, 1);
			glEnd();*/

			// Проверка по оси X
			if (thisFrontX >= otherMinX && thisFrontX <= otherMaxX)
				// Проверка по оси Y
				if (thisFrontY >= otherMinZ && thisFrontY <= otherMaxZ)
					return true;
		}

		return false;
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
	double velocity, points;
	bool frontBlock{ 0 }, backBlock{ 0 }, visible{ 1 };
	int dieAngl{ 0 };

	character(double width, double height, double depth, double posX, double posY, double posZ)
		: velocity(0), hitBox(width, height, depth, posX, posY, posZ){}

	int isCollidingVEC(std::vector<item> other) {
		double thisFrontX = posX + height / 2.0 * cos(angl / 57. - PI / 2.),
			thisFrontY = posZ + height / 2.0 * sin(angl / 57. - PI / 2.);

		double otherMinX{ 0 }, otherMaxX{ 0 }, otherMinZ{ 0 }, otherMaxZ{ 0 };

		for (int i{ 0 }; i < other.size(); i++) {
			otherMinX = other[i].posX - other[i].width / 2.;
			otherMaxX = other[i].posX + other[i].width / 2.;
			otherMinZ = other[i].posZ - other[i].height / 2.;
			otherMaxZ = other[i].posZ + other[i].height / 2.;

			//отображение точек хитбокса
			//glPointSize(5.0);
			//glColor3f(1.0, 0.0, 0.0);
			//glBegin(GL_POINTS);
			//glVertex3d(thisFrontX, thisFrontY, 1);
			//glEnd();

			// Проверка по оси X
			if (thisFrontX >= otherMinX && thisFrontX <= otherMaxX)
				// Проверка по оси Y
				if (thisFrontY >= otherMinZ && thisFrontY <= otherMaxZ)
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

//старые координаты мыши
int mouseX = 0, mouseY = 0;

float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;
bool stop{0};

//обработчик движения мыши
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	} 


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0*dx/ogl->getWidth()/zoom;
		offsetY += 1.0*dy/ogl->getHeight()/zoom;
	}


	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y,60,ogl->aspect);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

//обработчик вращения колеса  мыши
void mouseWheelEvent(OpenGL *ogl, int delta)
{

	float _tmpZ = delta*0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}

ObjFile MFox, MApple, MTrash, MTree, MRock, MGrass, MFlower, MLowgrass;

Texture TFox, TApple, TTrash, TTree, TRock, TGrass, TFlower, TLowgrass;

const double scaleFox = 0.02; //коэф уменьшения модельки
character Fox(1, 2.5, 2, 0, 0, 0);

std::vector<lowModel> Flower, Lowgrass;

std::vector<item> Apple;
const int appleCount = 5;
const int updateApple = 10;
int updateTime = updateApple;

std::vector<item> Trash;
const int trashCount = 15;

hitBox Tree[3] = { hitBox(1, 1, 2, -8, 0, -6),
				hitBox(1, 1, 2, 4, 0, 2) ,
				hitBox(1, 1, 2, -3, 0, 8) };

hitBox Rock(8, 8, 8, 6, 2, -6);
fromTo Kaban(3, 3, 3);

int record{ 0 };



//обработчик нажатия кнопок клавиатуры
void keyDownEvent(OpenGL* ogl, int key) {
	if (OpenGL::isKeyPressed('R')) {
		Fox.alive();
		updateTime = updateApple;
		Time = 0;
		stop = false;
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
	
	


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

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

	tick_n = GetTickCount();
	tick_o = tick_n;

	rec.setSize(300, 100);
	rec.setPosition(10, ogl->getHeight() - 100-10);
	rec.setText("T - вкл/выкл текстур\nL - вкл/выкл освещение\nF - Свет из камеры\nG - двигать свет по горизонтали\nG+ЛКМ двигать свет по вертекали",0,0,0);

	int x, y, angl;
	for (int i{ 1 }; i <= appleCount; i++) {
		getRandXY(x, y);
		Apple.push_back(item(1, 1, 1, x, 0, y));
	}
	for (int i{ 1 }; i <= trashCount; i++) {
		getRandXY(x, y);
		Trash.push_back(item(1, 1, 1, x, 0, y));
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
	double x = camera.Getforward().X(),
		y = camera.Getforward().Y();

	normalize(x, y); //делаем единичный вектор

	Fox.velocity = 4 * scaleFox;

	if (!Fox.dieAngl) {
		if (!Fox.frontBlock)
			if (OpenGL::isKeyPressed('W')) {
				if ((Fox.isColliding(Tree, 2) || Fox.isColliding(&Rock, 0)) && !(Fox.frontBlock || Fox.backBlock)) {
					Fox.frontBlock = 1;
					Fox.backBlock = 0;
				}
				else {
					Fox.frontBlock = 0;
					Fox.backBlock = 0;
				}
				Fox.posZ += y * Fox.velocity;
				Fox.posX += x * Fox.velocity;
				Fox.angl = camera.fi1 * 57 - 90;
			}

		if (!Fox.backBlock)
			if (OpenGL::isKeyPressed('S')) {
				if ((Fox.isColliding(Tree, 2) || Fox.isColliding(&Rock, 0)) && !(Fox.frontBlock || Fox.backBlock)) {
					Fox.frontBlock = 0;
					Fox.backBlock = 1;
				}
				else {
					Fox.frontBlock = 0;
					Fox.backBlock = 0;
				}
				Fox.posZ -= y * Fox.velocity;
				Fox.posX -= x * Fox.velocity;
				Fox.angl = camera.fi1 * 57 + 90;
			}
	}

	if (Fox.isCollidingVEC(Trash) 
		|| fabs(Fox.posX) > mapWH 
		|| fabs(Fox.posZ) > mapWH
		|| Fox.isColliding(&Kaban, 0)
		||  Fox.dieAngl > 0)
		if (Fox.dieAngl != 45) {
			Fox.dieAngl++;
			stop = true;
			record = Fox.points > record ? Fox.points : record;
		}
		else
			Fox.visible = 0;

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
	if (!stop) {
		tick_o = tick_n;
		tick_n = GetTickCount();
		Time += (tick_n - tick_o) / 1000.0;
	}

	if(Time >= updateTime){ //проверка времени респавна яблок
		int x, y;
		Apple.clear();
		for (int i{ 1 }; i <= appleCount; i++) {
			getRandXY(x, y);
			Apple.push_back(item(1, 1, 1, x, 0, y));
		}
		updateTime += updateApple;
	}

	if (Time >= updateTime - 5)
		Kaban.visible = true;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);


	kabanGo();
	move();
	

	s[0].UseShader();

	//передача параметров в шейдер.  Шаг один - ищем адрес uniform переменной по ее имени. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//Шаг 2 - передаем ей значение
	glUniform3fARB(location, light.pos.X(), light.pos.Y(), light.pos.Z());

	location = glGetUniformLocationARB(s[0].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[0].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[0].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[0].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[0].program, "md");
	glUniform3fARB(location, 0.4, 0.65, 0.5);

	location = glGetUniformLocationARB(s[0].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[0].program, "camera");
	glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());


	//хит боксы
	//RenderHitBox(Fox,1);
	//RenderHitBox(Apple,1);
	//RenderHitBox(Trash,1);
	//RenderHitBox(Tree[0],1);
	//RenderHitBox(Tree[1],1);
	//RenderHitBox(Tree[2],1);
	//RenderHitBox(Rock, 1);
	RenderHitBox(Kaban, 1);

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

	s[1].UseShader();
	int l = glGetUniformLocationARB(s[1].program, "rock");
	glUniform1iARB(l, 0);

	//Скала
	PUSH;
		glRotated(90, 1.0, 0.0, 0.0);
		glTranslated(Rock.posX, Rock.posY, -Rock.posZ);
		glScaled(5, 5, 5);
		TRock.bindTexture();
		MRock.DrawObj();
	POP;

	l = glGetUniformLocationARB(s[1].program, "tree");
	glUniform1iARB(l, 0);

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

	l = glGetUniformLocationARB(s[1].program, "carrot");
	glUniform1iARB(l, 0);

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
	for(auto& Apple : Apple){
		l = glGetUniformLocationARB(s[1].program, "Rapple");
		glUniform1iARB(l, 0);

		PUSH;
			glRotated(90, 1.0, 0.0, 0.0);
			glTranslated(Apple.posX, Apple.posY, -Apple.posZ);
			glScaled(0.1, 0.1, 0.1);
			TApple.bindTexture();
			MApple.DrawObj();
		POP;
	}

	//лиса
	if (Fox.visible) {
		l = glGetUniformLocationARB(s[1].program, "Lowpoly_Fox");
		glUniform1iARB(l, 0);

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
	Shader::DontUseShaders();
	
}   //конец тела функции


bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
void RenderGUI(OpenGL * ogl)
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
	rec.setSize(400, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "Переродиться - R" << std::endl << std::endl;
	ss << "Коорд. лисы: (" << Fox.posX << ", " << Fox.posZ << ", " << Fox.posY << ")" << " angl = " << (int)Fox.angl % 360 << std::endl;
	ss << "Коорд. кабана: (" << Kaban.posX << ", " << Kaban.posZ << ", " << Kaban.posY << ")" << " angl = " << (int)Kaban.angl % 360 << std::endl;
	ss << "Очки: " << Fox.points << std::endl;
	ss << "Time: " << Time << std::endl;
	ss << "~~~~~~~~~~~~~" << std::endl;
	ss << "Рекорд: " << record << std::endl;
	rec.setText(ss.str().c_str());
	rec.Draw();

	Shader::DontUseShaders();

}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

