#include "ObjLoader.h"


#include <string>
#include <vector>


using namespace std;

//Ищет первое в строке число дабл, с текущей позиции
//возвращает 1 при усшпешном конвертировании
//           0 - число не найдено
//после выполнения pos указывает на первый символ после найденого числа



string __digits1 = "0123456789-.";
int ReadDouble(string &s, int *pos, double *value)
{
	
	
	int start_pos = s.find_first_of(__digits1, *pos);
	int end_pos = s.find_first_not_of(__digits1, start_pos);
	
	if (start_pos == string::npos)
		return 0;
	if (end_pos == string::npos)
		end_pos = s.size();
	string dbl_str = s.substr(start_pos, end_pos - start_pos );
	*value = atof(dbl_str.c_str());
	*pos = end_pos;
	return 1;
}
string __digits2 = "0123456789";
int ReadUInt(string &s, int *pos, unsigned int *value)
{
	

	int start_pos = s.find_first_of(__digits2, *pos);
	int end_pos = s.find_first_not_of(__digits2, start_pos);

	if (start_pos == string::npos)
		return 0;
	if (end_pos == string::npos)
		end_pos = s.size();
	string dbl_str = s.substr(start_pos, end_pos - start_pos);
	*value = atol(dbl_str.c_str());
	*pos = end_pos;
	return 1;
}




int loadModel(char *filename, ObjFile *file)
{

	
	char *buf;
	long buf_size;
	

	//открываем файл
	HANDLE _file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	LARGE_INTEGER size;
	//узнаем размер файла
	GetFileSizeEx(_file, &size);

	buf_size = size.LowPart;
	 	
	buf = new char[buf_size];
	
	DWORD nBytesRead = 0;
	
	ReadFile(_file, buf, buf_size, &nBytesRead, 0);
	CloseHandle(_file);
	
	
	//vector<string> objStrings;
		
	char *cur;
	char *new_cur;

	cur = buf;

	vector<ObjVertex> V;
	vector<ObjTexCord> VT;
	vector<ObjNormal> VN;
	vector<ObjFace> F;

	bool isHaveTexCoords = false;
	bool isHaveNormals = false;
	string str;
	
	DWORD tick2 = GetTickCount();

	
	while (1)
	{
		
		new_cur = strstr(cur,"\n");
		if (!new_cur)
			new_cur = buf+buf_size;
		int strsize = new_cur - cur;		
		
		if (strsize > 0)
		{
			str.clear();
			str.append(cur, strsize);
			//objStrings.push_back(_new);
			while (1)
			{
				size_t pos = 0;

				if (str.find("v ") == 0) //описание вершины
				{
					DWORD tick1 = GetTickCount();
					int pos = 2;
					ObjVertex vert;
					double v;
					if (ReadDouble(str, &pos, &v)) //x
						vert.x = v;
					else return -1;

					if (ReadDouble(str, &pos, &v)) //y
						vert.y = v;
					else return -2;

					if (ReadDouble(str, &pos, &v)) //z
						vert.z = v;
					else return -3;

					if (ReadDouble(str, &pos, &v)) //w   =  1 def
						vert.w = v;
					vert.w = 1;

					V.push_back(vert);

					break;
				}

				if (str.find("vt ") == 0) //описание текст. координат
				{
					DWORD tick1 = GetTickCount();
					
					int pos = 2;
					ObjTexCord tex;
					double v;

					if (ReadDouble(str, &pos, &v)) //u
						tex.u = v;
					else return -4;

					if (ReadDouble(str, &pos, &v)) //v
						tex.v = v;
					else return -5;

					if (ReadDouble(str, &pos, &v)) //w   =   0   def
						tex.w = v;
					tex.w = 0;

					VT.push_back(tex);


					break;
				}

				if (str.find("vn ") == 0) //описание нормалей
				{
					DWORD tick1 = GetTickCount();
					
					int pos = 2;
					double v;
					ObjNormal norm;

					if (ReadDouble(str, &pos, &v)) //N x
						norm.x = v;
					else return -6;

					if (ReadDouble(str, &pos, &v)) //N y
						norm.y = v;
					else return -7;

					if (ReadDouble(str, &pos, &v)) //N z
						norm.z = v;
					else return -8;

					VN.push_back(norm);


					break;
				}

				if (str.find("f ") == 0) //описание граней
				{
					DWORD tick1 = GetTickCount();

					bool isHaveTexCoords = false;
					bool isHaveNormals = false;

					unsigned int d1;  //временный буфер для чтения числа
					int pos = 1;
					vector<unsigned int> Vertexes;
					vector<unsigned int> TexCoords;
					vector<unsigned int> Normals;

					unsigned int v = 0, t = 0, n = 0;
					
					file->Faces.push_back(ObjFace());
					std::list<ObjFace>::reverse_iterator it = file->Faces.rbegin();
						
					while (ReadUInt(str, &pos, &d1))
					{
						v = d1;/////   точка
						t = 0;
						n = 0;
						it->vertex.push_back(V[v-1]);
						

						if (str[pos] == ' ')
							break;
						if (str[pos] == '/' && str[pos + 1] == '/')
							if (ReadUInt(str, &pos, &d1))  ///////////   нормаль без текстуры
								n = d1;
							else return -9;
						else if (str[pos] == '/')
						{
							if (ReadUInt(str, &pos, &d1))///////////   читаем текстуру
								t = d1;
							else return -10;

							if (str[pos] == ' ')
								break;

							if (ReadUInt(str, &pos, &d1))///////////   нормаль, при условии наличия текстуры.
								n = d1;
							else return -11;
						}

						if (n != 0)
						{
							isHaveNormals = true;
							it->normal.push_back(VN[n - 1]);
						}

						if (t != 0)
						{
							isHaveTexCoords = true;
							it->texCoord.push_back(VT[t - 1]);
						}


						if (isHaveNormals && (Vertexes.size() != Normals.size()) ||
							isHaveTexCoords&&Vertexes.size() != TexCoords.size())
							return -12;  //неодинаковое описание вершины!! ошибка		

					}
					
					break;
				}

				break;
			}


		}
		cur = new_cur + 1;

		if (cur > (buf+buf_size))
			break;		
	}	
	delete buf;
	

	glDeleteLists(file->listId, 1);
	file->listId = glGenLists(1);

	glNewList(file->listId, GL_COMPILE);
	file->RenderModel(GL_POLYGON);
	glEndList();

	return 1;



}




