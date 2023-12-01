#ifndef MYSHADERS_H
#define MYSHADERS_H

#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include <string>


extern PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
extern PFNGLGETHANDLEARBPROC glGetHandleARB;
extern PFNGLDETACHOBJECTARBPROC glDetachObjectARB;
extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
extern PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
extern PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
extern PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
extern PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
extern PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB;
extern PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;

extern PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
extern PFNGLUNIFORM1FARBPROC glUniform1fARB;
extern PFNGLUNIFORM1IARBPROC glUniform1iARB;
extern PFNGLUNIFORM3FARBPROC glUniform3fARB;
extern PFNGLUNIFORM2FARBPROC glUniform2fARB;
extern PFNGLUNIFORM4FARBPROC glUniform4fARB;
extern PFNGLUNIFORM1IARBPROC glUniform1iARB;

extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;

extern PFNGLACTIVETEXTUREPROC glActiveTexture;



void initShadersFunctions();
int loadShader(const char  *filename, char **shaderSrc, int *programLength);







//вызываем эту функцию в  перед рендером для настройки шейдеров

void initShadersFunctions();

class Shader
{
protected:


public:
	GLhandleARB program;
	GLhandleARB vertex;
	GLhandleARB fragment;

	std::string VshaderFileName;
	std::string FshaderFileName;



	Shader()
	{


	}

	~Shader()
	{

	}

	void LoadShaderFromFile();


	void Compile();


	void UseShader();

	static void DontUseShaders();
};


//загружает код шейдера из файла
//************************************
// Parameter: char * filename   имя файла
// Parameter: char * * shaderSrc   указатель на массив символов, в который загрузится код шейдера. после того, как станет ненужным надо высвобадить память!
// Parameter: int * programLength  указатель на длину массива символов.
//************************************
int loadShader(const char  *filename, char **shaderSrc, int *programLength);


#endif // !MYSHADERS_H
