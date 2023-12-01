#ifndef GUITEXTRECTANGLE_H
#define GUITEXTRECTANGLE_H


class GuiTextRectanglePrivate;

//Прямоугольник с текстом
class GuiTextRectangle
{
protected:
	GuiTextRectanglePrivate *d_ptr;
	GuiTextRectanglePrivate* d_func(){ return d_ptr; }
	void createTexture();

public:
	GuiTextRectangle();
	virtual ~GuiTextRectangle();

	void setSize(int width, int height);
	int getWidth();
	int getHeight();

	void setPosition(int x, int y);

	void setText(const char* text, char r = 0, char g = 0, char b = 0);

	void Draw();
};


#endif