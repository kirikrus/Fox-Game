#include "MyShaders.h"


PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
PFNGLGETHANDLEARBPROC glGetHandleARB;
PFNGLDETACHOBJECTARBPROC glDetachObjectARB;
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
PFNGLCOMPILESHADERARBPROC glCompileShaderARB;
PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB;
PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
PFNGLUNIFORM1FARBPROC glUniform1fARB;
PFNGLUNIFORM1IARBPROC glUniform1iARB;
PFNGLUNIFORM3FARBPROC glUniform3fARB;
PFNGLUNIFORM2FARBPROC glUniform2fARB;
PFNGLUNIFORM4FARBPROC glUniform4fARB;
PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
PFNGLACTIVETEXTUREPROC glActiveTexture;

int loadShader(const char *filename, char **shaderSrc, int *programLength)
{
	
		//открываем файл
		HANDLE file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		LARGE_INTEGER size;

		//узнаем размер файла
		GetFileSizeEx(file, &size);

		*programLength = (int)size.LowPart;

		//выделяем память под буфер для считывания шейдера
		*shaderSrc = (char *)malloc(*programLength*sizeof(char));
		int sss = *programLength*sizeof(char);

		DWORD nBytesRead = 0;
		//считываем шейдер  в буфер
		ReadFile(file, *shaderSrc, *programLength, &nBytesRead, 0);
		CloseHandle(file);
		return 1;
	

}
bool init = false;
void initShadersFunctions()
{
	//наполняем делегаты адресами на функции
	glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
	glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
	glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
	glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)wglGetProcAddress("glDetachObjectARB");
	glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
	glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
	glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
	glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
	glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)wglGetProcAddress("glValidateProgramARB");
	glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
	glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
	glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");


	glUniform1fARB = (PFNGLUNIFORM1FARBPROC)wglGetProcAddress("glUniform1fARB");
	glUniform1iARB = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
	glUniform3fARB = (PFNGLUNIFORM3FARBPROC)wglGetProcAddress("glUniform3fARB");
	glUniform4fARB = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
	glUniform2fARB = (PFNGLUNIFORM2FARBPROC)wglGetProcAddress("glUniform2fARB");
	glUniform1iARB = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
	glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
	

	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");

	init = true;
}

void Shader::LoadShaderFromFile()
{
	if (!init)
		initShadersFunctions();

	glDeleteObjectARB(program);
	glDeleteObjectARB(vertex);
	glDeleteObjectARB(fragment);

	program = glCreateProgramObjectARB();
	vertex = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fragment = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);


	char *shader1Text;
	char *shader2Text;
	int shader1TextSize;
	int shader2TextSize;

	loadShader(FshaderFileName.c_str(), &shader1Text, &shader1TextSize);
	loadShader(VshaderFileName.c_str(), &shader2Text, &shader2TextSize);

	const char *_f = shader1Text;
	const char *_v = shader2Text;

	glShaderSourceARB(fragment, 1, &_f, &shader1TextSize);
	glShaderSourceARB(vertex, 1, &_v, &shader2TextSize);


	free(shader1Text);
	free(shader2Text);
}

void Shader::Compile()
{
	
		
	
	   
	//компиляция фрагментного шейдера
	    glCompileShaderARB(fragment);

		{
			//Получение сообщения о ошибках компиляции
			int compiled = 0; int length = 0; int laux = 0;
			glGetObjectParameterivARB(fragment, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
			glGetObjectParameterivARB(fragment, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
			char *log = new char[length];
			glGetInfoLogARB(fragment, length, &laux, log);
			if (!compiled)
				MessageBox(0, log, "fragment shader error!", 0);
			delete log;
		}

		glCompileShaderARB(vertex);
		{
			int compiled = 0; int length = 0; int laux = 0;

			glGetObjectParameterivARB(vertex, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
			glGetObjectParameterivARB(vertex, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
			char *log = new char[length];
			glGetInfoLogARB(vertex, length, &laux, log);
			if (!compiled)
				MessageBox(0, log, "vertex shader error!", 0);
			delete log;
		}

		//привязывем шейдер к программному объекту
		glAttachObjectARB(program, fragment);
		glAttachObjectARB(program, vertex);
		
		//линкуем
		glLinkProgramARB(program);
		{
			int result = 0; int length = 0; int laux = 0;
			glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB, &result);
			glGetObjectParameterivARB(program, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
			char *log = new char[length];
			glGetInfoLogARB(program, length, &laux, log);
			if (!result)
				MessageBox(0, log, "fragment shader error!", 0);
			delete log;
		}
		

	
}

void Shader::UseShader()
{
	glUseProgramObjectARB(program);	
}

void Shader::DontUseShaders()
{
	glUseProgramObjectARB(0);	
}
