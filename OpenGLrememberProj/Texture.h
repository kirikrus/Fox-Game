#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <windows.h>
#include <GL\GL.h>

class Texture
{
public: 
	~Texture();

	unsigned int texId;

	void loadTextureFromFile(const char* filename);
	void deleteTexture();

	void bindTexture();
	
	static int Texture::LoadBMP(__in LPCSTR  filename, __out int* Wigth, __out int *Height, __out RGBTRIPLE **arr);
	static int RGBtoChar(__in RGBTRIPLE *arr, __in int width, __in int height, __out char **out);
};


#endif //TEXTURE_H