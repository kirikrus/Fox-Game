#ifndef CAMERA_H
#define CAMERA_H

#include "Object.h"
#include "MyVector3d.h"
#include "Ray.h"


//интерфейс для доступа к камере
class Camera : public Object
{
public:

	Vector3 lookPoint;
	Vector3 normal;

	Camera()
	{

	}

	virtual void LookAt()
	{
		
	}

	virtual void SetUpCamera()
	{

	}

	Ray getLookRay(int wndX, int wndY, double FOV, double aspect)
	{
		GLint    viewport[4];    // параметры viewport-a.
		GLdouble projection[16]; // матрица проекции.
		GLdouble modelview[16];  // видовая матрица.
//		GLdouble vx, vy, vz;       // координаты курсора мыши в системе координат viewport-a.
		GLdouble wx, wy, wz;       // возвращаемые мировые координаты.

		//настраиваем камеру на преспективу для корретного чтения координат
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(FOV,aspect, .1, 100.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		LookAt();

		glGetIntegerv(GL_VIEWPORT, viewport);           // узнаём параметры viewport-a.
		glGetDoublev(GL_PROJECTION_MATRIX, projection); // узнаём матрицу проекции.
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);   // узнаём видовую матрицу.
		// переводим оконные координаты курсора в систему координат viewport-a.
				 
		gluUnProject(wndX, wndY, 0, modelview, projection, viewport, &wx, &wy, &wz);
		Vector3 o(wx, wy, wz);
		gluUnProject(wndX, wndY, 1, modelview, projection, viewport, &wx, &wy, &wz);
		Vector3 p(wx, wy, wz);
		Ray r;
		r.origin = o;
		r.direction = (p - o).normolize();
		return r;

	}
};

#endif