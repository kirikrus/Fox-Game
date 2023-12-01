#ifndef PRIMITIVESSTATIC_H
#define PRIMITIVESSTATIC_H

#include <vector>
#include "MyVector3d.h"

class Circle;
class Sphere;
class OpenGL;

class CircleStatic
{
	static std::vector <Vector3> points;
	static void MakeVertex();

	friend Circle;
	friend OpenGL;
};

class SphereStatic
{
	static std::vector <Vector3> points;
	static void MakeVertex();

	friend Sphere;
	friend OpenGL;
};

#endif