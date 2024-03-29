﻿/*
* Aluno: Caio Henrique Martins dos Santos
* Disciplina: Construção de Software Gráfico 3D
* Data: 02/dez/2020
*
* CG - Trabalho Final
*
* Utilizado Visual Studio 2019
* Platform Toolset Visual Studio 2015 (v140)
* Target Platform Version 10.0.18362.0
*
* Requer C++11
*/

#include <windows.h>  
#include <GL\freeglut.h>  
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <fstream>		    
#include <iostream>
#include <vector>
#include <functional>

using namespace std;

char title[128] = "OpenGL-PUCPR - Formas geométricas";
char ver[8] = "1.3.3";

const char filename[] = "df.txt";

void renderWorld();
void draw2dBox(int, int, int, int);
void draw2dBoxFilled(int, int, int, int);
void renderStrokeString(float, float, const char*, float, bool, void*);
void renderString(float, float, void*, const char*);
void keyboardTextInput(unsigned char, int, int);
void resumeButton();
void displayFileLoad();
void displayFileSave();
void quit();

class ObjetoOpenGL
{
	/* Representa um objeto individual como um cubo, cone, teapot */
public:
	int tipo, id = 0;
	//bool selected;
	double x, y, z; /* Posição */
	float rX, rY, rZ, r, g, b; /* Rotação e cor */
	std::vector<float> params; /* Vetor de parâmetros adicionais */

	ObjetoOpenGL(int Tipo, double X, double Y, double Z, float RX, float RY, float RZ, float R, float G, float B, std::vector<float> Parametros)
	{
		tipo = Tipo;
		x = X;
		y = Y;
		z = Z;
		rX = RX;
		rY = RY;
		rZ = RZ;
		r = R;
		g = G;
		b = B;
		params = Parametros;
	}

	ObjetoOpenGL() {}
};

class ObjetoCompostoOpenGL
{
	/*
	Representa um objeto composto, selecionável pelas teclas 0-9 no teclado.
	Possui um nome em texto definido no displayfile e um vetor de partes do tipo ObjetoOpenGL que
	compõem a cena.
	*/
public:
	char nome[50];
	std::vector<ObjetoOpenGL> partes;

	ObjetoCompostoOpenGL(char Nome[50], std::vector<ObjetoOpenGL> Partes = {})
	{
		strcpy_s(nome, Nome);
		partes = Partes;
	}

	ObjetoCompostoOpenGL() {}
};

class Botao
{
	/* Objeto Botão que o usuário pode clicar, executando um método F()*/
public:
	char nome[50];
	int w, h; /* Posição do botão em relação à borda esquerda superior da tela*/
	std::function<void()> f; /* Método armazenado executado on click */

	Botao(const char* Nome, int W, int H, void F())
	{
		strcpy_s(nome, Nome);
		w = W;
		h = H;
		f = F;
	}

	void desenharBotao(int x, int y, int mouseX, int mouseY, bool lClick, bool strokeString = true, void* font = GLUT_STROKE_MONO_ROMAN)
	{

		/* Floats definidos abaixo posicionam o botão com centro na coordenada x,y da viewport */
		float
			buttonMidX = w / 2,
			buttonMidY = h / 2,
			leftX = x - buttonMidX,
			bottomY = y - buttonMidY,
			rightX = x + buttonMidX,
			topY = y + buttonMidY;


		char buffer[20];
		snprintf(buffer, sizeof(buffer), nome);

		glPolygonMode(GL_FRONT, GL_FILL);

		glColor3f(0.1, 0.1, 0.1);
		draw2dBoxFilled(leftX, bottomY, rightX, topY);

		if (leftX < mouseX && mouseX < rightX && bottomY < mouseY && mouseY < topY)
		{
			/* Mouse sobre o botão? */
			glColor3f(0.0, 1.0, 0.0);
			draw2dBox(leftX, bottomY, rightX, topY);  /* Desenha outline verde */
			if (lClick) f(); /* Executa f() */
		}

		glColor3f(1, 1, 1); /* Texto branco */

		if (strokeString)
		{
			renderStrokeString(x + 9, bottomY + 9, buffer, 0.22, true, font);
		}
		else
		{
			renderString(leftX + 5, bottomY + 1, font, buffer);
		}
	}
};

class MenuEsc
{
	/* Menu com opções que aparece quando o usuário aperta ESC */
public:
	std::vector<Botao> botoes;
	int bw = 300;
	int bh = 40;

	MenuEsc() {}

	void addBotao(const char* Nome, void F())
	{
		char nome[50];
		strcpy_s(nome, Nome);
		Botao b(Nome, bw, bh, F);
		this->botoes.push_back(b);
	}

	void desenharMenu(int windowW, int windowH, int mouseX, int mouseY, bool lClick)
	{
		for (size_t i = 0; i < botoes.size(); i++)
		{
			int top = (int)(windowH / 2) - 10 - botoes.size() * bh / 2; // Determina a menor posição Y, topo do primeiro botão
			botoes[i].desenharBotao(windowW / 2, top + (bh + 10) * i, mouseX, windowH - mouseY, lClick);
		}
	}

	void inicializarMenu()
	{
		this->addBotao("Sair", quit);
		this->addBotao("Carregar", displayFileLoad);
		this->addBotao("Salvar", displayFileSave);
		this->addBotao("Continuar", resumeButton);
	}
};

GLint viewport[4];

GLuint selectBuffer[1000];
GLint hits;

GLfloat
nRange = 120.0f,
angleV = 70.0f,
fAspect,
zNear = 0.01,
zFar = 10000,
angleX = 0.0f,
angleY = 0.0f,
angleZ = 0.0f, // Arrow keys user-defined rotation
rotX = 0.0f,
rotY = 0.0f,
rotZ = 0.0f, // The final global rotation (with added animation)
mouseZ;

bool
animate,
animateX,
animateY,
animateZ,
polygonMode,
front,
back,
cface,
projMode,
moving,
globalIllumination = true,
depthTest = true,
shadeModel = true,
mKey = true,
slashKey = true,
wKey, aKey, sKey, dKey, spaceKey, eKey,
upKey, leftKey, rightKey, downKey, pgDnKey,
pgUpKey, rClick, lClick, escKey, speedModifierBool;

int
parteIdx = 0,
forma = 1,
frame = 0,
mouseX, // live mouse position X axis updated every frame
mouseY, // live mouse position Y axis
rClickX, // mouse right click X position updates on right click and/or right click dragging
rClickY, // mouse right click Y position
lClickX, // mouse left click X position updates on left click (no dragging)
lClickY, // mouse left click Y position
mouseMovedX, // camera movement mouse variables
mouseMovedY,
lastX,
lastY,
idSelecionado,
parteSelecionada;

float
rAngle,
time,
time1,
framerate,
frametime,
lasttime,
calculatedFramerate,
calculatedFrametime,
versorVisionX,
versorVisionY,
versorVisionZ,
visionMag,
cameraPitch = 0.0f,
cameraYaw = 270.0f,
lookingAtX = 0,
lookingAtY = 0,
lookingAtZ = 0,
visionX = 0,
visionY = 0,
visionZ = 0,
xPos = 0,
yPos = 0,
zPos = 20,
speed = 1.0f,
deltaPosicaoX = 0.0f,
deltaPosicaoY = 0.0f,
deltaPosicaoZ = 0.0f,
objMovX, objMovY, objMovZ,
lastObjMovX, lastObjMovY, lastObjMovZ,
cameraSensitivity = 0.1f;

GLdouble
winX,
winY,
winZ,
objX,
objY,
objZ,
Mmodelview[16],
Mprojection[16];

ObjetoCompostoOpenGL objetoCompostoAtual;
ObjetoOpenGL objSelecionado;

std::vector<ObjetoCompostoOpenGL> Objetos;
std::vector<ObjetoOpenGL> Retas;
MenuEsc menuEsc;

std::string dialogString = "";
std::string inputString = "";

void displayFileLoad()
{
	/*	Formato arquivo:
	xPos yPos zPos lookingAtX lookingAtY lookingAtZ cameraPitch cameraYaw;
	numObjects
	nome
	numPartes
	tipo x y z r g b numParametros parametro[0] parametro[...] parametro[numParametros-1]  (Por exemplo aresta do cubo, raio/comprimento de um cilindro)
	nome (Próximo objeto)
	...
	*/
	int numObjects, numPartes;
	char nome[50];
	int tipo, numParametros;
	float x, y, z, rX, rY, rZ, r, g, b;
	std::vector<float> parametros;  // cores dos objetos
	Objetos.clear();

	fstream inStream;
	inStream.open(filename, ios::in); // abre o arquivo
	if (inStream.fail()) return;      //falha na abertura do arquivo
	cout << "Abertura do arquivo com sucesso ..." << endl;
	inStream >> xPos >> yPos >> zPos >> lookingAtX >> lookingAtY >> lookingAtZ >> cameraPitch >> cameraYaw;
	inStream >> numObjects;			  // le primeira linha do arquivo, numero de objetos 
	cout << numObjects << " Objetos na cena ..." << endl;

	for (int i = 1; i <= numObjects; i++) // leitura das demais linhas, ID dos objetos, posicao e cor
	{
		inStream >> nome; // Recebe nome
		ObjetoCompostoOpenGL novoObjeto(nome); // Cria objeto composto com nome recebido
		inStream >> numPartes; // Recebe numero de partes que compoem o objeto
		for (int j = 1; j <= numPartes; j++) // Para cada parte
		{
			parametros.clear();
			inStream >> tipo >> x >> y >> z >> rX >> rY >> rZ >> r >> g >> b >> numParametros; // Lê um objeto open gl (uma parte)
			for (int k = 0; k < numParametros; k++) // Para cada parametro adicional, le uma linha nova com cada parametro
			{
				float p;
				inStream >> p;
				parametros.push_back(p);
			}
			ObjetoOpenGL parte(tipo, x, y, z, rX, rY, rZ, r, g, b, parametros); // Cria a parte
			//parte.distribuirParametros(parametros); // distribui os parametros lidos para as variaveis dentro do objeto 'parte'
			novoObjeto.partes.push_back(parte); // mexendo na variável pública para fins de teste
		}
		Objetos.push_back(novoObjeto);
	}
	inStream.close();				 // fecha o arquivo
}

void displayFileSave()
{
	cout << "File save iniciado" << endl;
	ofstream file;
	file.open(filename);
	file << xPos << ' ' << yPos << ' ' << zPos << ' ' << lookingAtX << ' ' << lookingAtY << ' ' << lookingAtZ << ' ' << cameraPitch << ' ' << cameraYaw << endl;
	file << Objetos.size() << endl;
	cout << "Salvando " << Objetos.size() << " objetos compostos." << endl;

	for (ObjetoCompostoOpenGL o : Objetos)
	{
		cout << "Objeto " << o.nome << " Possui " << o.partes.size() << " partes" << endl;
		file << o.nome << endl << o.partes.size() << endl;
		for (ObjetoOpenGL p : o.partes)
		{
			file << p.tipo << ' ' << p.x << ' ' << p.y << ' ' << p.z << ' ' << p.rX << ' ' << p.rY << ' ' << p.rZ << ' ' << p.r << ' ' << p.g << ' ' << p.b << ' ' << p.params.size() << ' ';
			for (float param : p.params)
			{
				file << param << ' ';
			}
			file << endl;
		}
	}
	file.close();
}

void initGL()
{
	//glutSetOption(GLUT_GEOMETRY_VISUALIZE_NORMALS, 1);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClearDepth(1.0f);                   // Set background depth to farthest
	//glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
	glShadeModel(GL_SMOOTH);     // Enable smooth shading
	//glEnable(GL_NORMALIZE);
	glSelectBuffer(100, selectBuffer);
}

float toRadians(float angle)
{
	return angle * M_PI / 180;
}

float toDegrees(float radianAngle)
{
	return radianAngle / (M_PI / 180);
}

void renderString(float x, float y, void* font, const char* string)
{

	glRasterPos2f(x, y);
	glutBitmapString(font, (unsigned char*)string);
}

void renderString3D(float x, float y, float z, void* font, const char* string)
{

	glRasterPos3f(x, y, z);
	glutBitmapString(font, (unsigned char*)string);
}

void renderStrokeString(float x, float y, const char* string, float scale, bool centered = false, void* font = GLUT_STROKE_MONO_ROMAN)
{
	unsigned char* s = (unsigned char*)string;

	if (centered)
	{
		glColor3f(1.0, 1.0, 1.0);
		glTranslatef(x - (glutStrokeLength(font, s) + glutStrokeWidth(font, 'a')) / 2 * scale, y, 10);
		glScalef(scale, scale, scale);
		glutStrokeString(font, s);
		glLoadIdentity();
	}
	else
	{
		glTranslatef(x - glutStrokeLength(font, s) / 2 * scale, y, -10);
		glScalef(scale, scale, scale);
		glutStrokeString(font, s);
		glLoadIdentity();
	}
}

void processSpecialKeys(int key, int x, int y)
{
	//printf("%d   -   %d, %d\n", key, x, y);
	switch (key) {
	case GLUT_KEY_LEFT:
		leftKey = true;
		break;
	case GLUT_KEY_RIGHT:
		rightKey = true;
		break;
	case GLUT_KEY_UP:
		upKey = true;
		break;
	case GLUT_KEY_DOWN:
		downKey = true;
		break;
	case GLUT_KEY_PAGE_UP:
		pgUpKey = true;
		break;
	case GLUT_KEY_PAGE_DOWN:
		pgDnKey = true;
		break;
	case GLUT_KEY_F11:
		printf("Alternado entre Windowed/Fullscreen");
		glutFullScreenToggle();
		break;
	case GLUT_KEY_F1:
		printf("Carregando arquivo...");
		displayFileLoad();
		break;

	case GLUT_KEY_F2:
		printf("Salvando arquivo...");
		displayFileSave();
		break;
	}
}

void processSpecialKeysUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		leftKey = false;
		break;
	case GLUT_KEY_RIGHT:
		rightKey = false;
		break;
	case GLUT_KEY_UP:
		upKey = false;
		break;
	case GLUT_KEY_DOWN:
		downKey = false;
		break;
	case GLUT_KEY_PAGE_UP:
		pgUpKey = false;
		break;
	case GLUT_KEY_PAGE_DOWN:
		pgDnKey = false;
		break;
	}
}

void processNormalKeys(unsigned char key, int x, int y)
{
	//printf("%c  -  %d, %d\n", key, x, y);
	switch (key)
	{
	case ';':
	case '/':
		slashKey = !slashKey;
		break;
	case 27: /* esc */
		escKey = !escKey;
		break;
	case 'r':
		Retas.clear();
		idSelecionado = 0;
		parteSelecionada = 0;
		rAngle = 0;
		angleX = 0;
		angleY = 0;
		angleZ = 0;
		rotX = 0;
		rotY = 0;
		rotZ = 0;
		break;
	case 'n':
		animate = !animate;
		break;
	case 'x':
		animateX = !animateX;
		break;
	case 'y':
		animateY = !animateY;
		break;
	case 'z':
		animateZ = !animateZ;
		break;
	case 'w':
		wKey = true;
		break;
	case 'a':
		aKey = true;
		break;
	case 's':
		sKey = true;
		break;
	case 'd':
		dKey = true;
		break;
	case ' ':
		spaceKey = true;
		break;
	case 'e':
		eKey = true;
		break;
	case 'v':
		speedModifierBool = !speedModifierBool;
		break;
	case 'm':
		mKey = !mKey;
		break;
	case 'f':
		front = !front;
		break;
	case 'b':
		back = !back;
		break;
	case 'c':
		cface = !cface;
		cface ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		break;
	case 't':
		depthTest = !depthTest;
		depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		break;
	case 'p': // p projection mode
		projMode = !projMode;
		break;
	case 'i':
		globalIllumination = globalIllumination ? false : true;
		break;
	case 'l':
		shadeModel = !shadeModel;
		shadeModel ? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);
		break;
	case '1':
		forma = 1;
		break;
	case '2':
		forma = 2;
		break;
	case '3':
		forma = 3;
		break;
	case '4':
		forma = 4;
		break;
	case '5':
		forma = 5;
		break;
	case '6':
		forma = 6;
		break;
	case '7':
		forma = 7;
		break;
	case '8':
		forma = 8;
		break;
	case '9':
		forma = 9;
		break;
	case '0':
		forma = 10;
		globalIllumination = false;
		front = true;
		break;
	}

	if (key >= 48 && key <= 57) // Mudança de objeto, desselecionar objeto // 0 a 9
	{
		idSelecionado = 0;
		parteSelecionada = 0;
	}
}

void processNormalKeysUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		wKey = false;
		break;
	case 'a':
		aKey = false;
		break;
	case 's':
		sKey = false;
		break;
	case 'd':
		dKey = false;
		break;
	case ' ':
		spaceKey = false;
		break;
	case 'e':
		eKey = false;
		break;
	}
}

unsigned int selecionarObjeto()
{
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPickMatrix(mouseX, viewport[3] - mouseY, 1.0, 1.0, viewport);
	gluPerspective(angleV, fAspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	renderWorld();
	glPopMatrix();
	hits = glRenderMode(GL_RENDER);
	if (!hits)
	{
		return 0;
	}
	//printf("%d hits\n", hits);
	//printf("selectBuffer[3] = %d\n", selectBuffer[3]);
	unsigned int menorDepth, menorDepthObj, menorDepthIndex = 1;
	menorDepth = selectBuffer[1];
	for (size_t i = 0; i < hits; i++)
	{
		unsigned int newDepth = selectBuffer[1 + i * 4];
		if (newDepth < menorDepth)
		{
			menorDepth = newDepth;
			menorDepthIndex = 1 + i * 4;
		}
	}
	menorDepthObj = selectBuffer[menorDepthIndex + 2];
	//printf("menorDepthIndex = %d obj = %d\n", menorDepthIndex, menorDepthObj);
	return menorDepthObj;
}

void desenharRaycast()
{
	/* Desenha reta de onde o usuário está até o ponto em zFar ou um objeto para onde seu mouse aponta */

	glReadPixels(mouseX, viewport[3] - mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &mouseZ);
	gluUnProject(mouseX, viewport[3] - mouseY, mouseZ, Mmodelview, Mprojection, viewport, &objX, &objY, &objZ);
	printf("%f, %f, %f\n", (float)objX, (float)objY, (float)objZ);

	std::vector<float> p = { xPos, yPos, zPos, (float)objX, (float)objY, (float)objZ };

	ObjetoOpenGL objReta(0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, p);

	Retas.emplace_back(objReta); /* Ao final adiciona ao vetor Retas*/
}

void mouse(int button, int state, int x, int y)
{
	if (button == 0) // left click
	{
		// Coordenadas são salvas e estado clicado é usado por outras funções
		if (rClick) return;
		if (state == GLUT_DOWN)
		{
			lClick = true;
			lastX = lClickX = x;
			lastY = lClickY = y;

			if (!(x > 245 && x < 400 && y > 380 && y < 600 && parteSelecionada || escKey))
			{
				// Mouse não está em cima do menu do objeto, e pode selecionar novos objetos
				idSelecionado = selecionarObjeto();
				if (idSelecionado == 0)
				{
					parteSelecionada = 0;
				}
				if (idSelecionado > 6) 
				{
					/*
					* Os ids de 1 a 6 são destinados aos objetos que fazem a movimentação/rotação (xyzLines3d e rotationTorus3d)
					* apenas modificamos qual objeto (parte) está selecionado caso o usuário escolha outro que não é um objeto 
					* com funcionalidade de movimentação.
					*/
					parteSelecionada = idSelecionado;
				}
			}
			//desenharRaycast(); /* Descomentar para habilitar ray casting em cada clique */
		}
		else
		{
			lClick = false;
			mouseMovedX = 0;
			mouseMovedY = 0;
		}
	}

	if (button == 2) // right click
	{
		if (lClick) return;
		if (state == GLUT_DOWN)
		{
			rClick = true;
			lastX = rClickX = x;
			lastY = rClickY = y;
		}
		else
		{
			rClick = false;
			mouseMovedX = 0;
			mouseMovedY = 0;
		}
	}

	if ((button == 3) || (button == 4)) // scroll wheel
	{
		if (state == GLUT_UP) return;
		// Scroll wheel muda nRange quando em projeção ortogonal e muda angleV quando em projeção perspectiva
		if (button == 3)
		{
			if (projMode)
			{
				if (nRange > 0.5)
				{
					nRange -= 0.5;
				}
			}
			else if (angleV > 0.5)
			{
				angleV -= 0.5;
			}
		}
		else
		{
			if (projMode)
			{
				nRange += 0.5;
			}
			else if (angleV < 179.5)
			{
				angleV += 0.5;
			}
		}
	}
}

void mouseMovement(int x, int y)
{
	mouseX = x;
	mouseY = y;
	mouseMovedX = x - lastX; // mouse position changes
	mouseMovedY = y - lastY;
	lastX = x;
	lastY = y;

	if (!lClick)
	{
		glReadPixels(mouseX, viewport[3] - mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &mouseZ);
	}
	gluUnProject(mouseX, viewport[3] - mouseY, mouseZ, Mmodelview, Mprojection, viewport, &objX, &objY, &objZ);

	objMovX = objX;
	objMovY = objY;
	objMovZ = objZ;

	deltaPosicaoX = objX - lastObjMovX;
	deltaPosicaoY = objY - lastObjMovY;
	deltaPosicaoZ = objZ - lastObjMovZ;

	lastObjMovX = objX;
	lastObjMovY = objY;
	lastObjMovZ = objZ;


	if (lClick) /* Quando o usuário clica com o botão esquerdo do mouse */
	{
		switch ((int)(idSelecionado)) /* Se ele selecionou um objeto com funcionalidade de movimentação, mudanças são aplicadas com movimento do mouse */
		{
		case 1:
			// Usuario selecionou movimentação no eixo X
			Objetos[forma - 1].partes[parteIdx].x += deltaPosicaoX * 10;
			break;
		case 2:
			// Usuario selecionou movimentação no eixo Y
			Objetos[forma - 1].partes[parteIdx].y += deltaPosicaoY * 10;
			break;
		case 3:
			// Usuario selecionou movimentação no eixo Z
			Objetos[forma - 1].partes[parteIdx].z += deltaPosicaoZ * 10;
			break;
		case 4:
			// Usuario selecionou movimentação no eixo de rotação rX
			Objetos[forma - 1].partes[parteIdx].rX += deltaPosicaoY * 10;
			break;
		case 5:
			// Usuario selecionou movimentação no eixo de rotação rY
			Objetos[forma - 1].partes[parteIdx].rY += deltaPosicaoY * 10;
			break;
		case 6:
			// Usuario selecionou movimentação no eixo de rotação rZ
			Objetos[forma - 1].partes[parteIdx].rZ += deltaPosicaoY * 10;
			break;
		default:
			break;
		}
	}

	if (rClick) /* Botão direito do mouse = movimentação da câmera */
	{
		if (!escKey) // bloqueia movimentação da câmera quando o menu ESC é aberto
		{
			cameraPitch -= mouseMovedY * cameraSensitivity; // pitch e yaw mudam de acorco com diferenças nas coordenadas do mouse
			cameraYaw += mouseMovedX * cameraSensitivity;
		}

		if (cameraPitch > 85)
		{
			cameraPitch = 85; // limitando pitch entre -85 and 85
		}
		if (cameraPitch < -85)
		{
			cameraPitch = -85;
		}

		cameraYaw = (cameraYaw > 360) ? (cameraYaw - 360) : (cameraYaw < 0) ? (cameraYaw + 360) : cameraYaw; // limitando yaw ao intervalo (0, 360)

		lookingAtX = xPos + cos(toRadians(cameraYaw)) * cos(toRadians(cameraPitch)); // câmera se move em um sistema de coordenadas esféricas
		lookingAtY = yPos + sin(toRadians(cameraPitch));
		lookingAtZ = zPos + sin(toRadians(cameraYaw)) * cos(toRadians(cameraPitch));
	}
}

void normalize2d(float x, float y, float* returnX, float* returnY)
{
	// funcao antiga mas deixarei para não ter que mexer no que ja esta funcionando
	float magnitude = sqrt(pow(x, 2) + pow(y, 2));

	*returnX = x / magnitude;
	*returnY = y / magnitude;
}

void normalizarVetor(float v[], size_t s, float* vOut)
{
	// Recebe um vetor v[] s-dimensional e retorna vetor unitário em vOut
	float mag = 0;

	float* nV = new float[s];

	for (size_t i = 0; i < s; i++) {
		mag += pow(v[i], 2);
	}

	mag = sqrt(mag);

	if (mag == 0)
	{
		mag = 1;
	}

	for (size_t i = 0; i < s; i++)
	{
		nV[i] = v[i] / mag;
	}

	memcpy(vOut, nV, sizeof(1.0f) * s);

	delete[] nV;
}

void calcNormal(float v[3][3], float out[3])
{
	float v1[3], v2[3], v3[3];
	static const int x = 0; static const int y = 1; static const int z = 2;

	v1[x] = v[0][x] - v[1][x];
	v1[y] = v[0][y] - v[1][y];
	v1[z] = v[0][z] - v[1][z];

	v2[x] = v[1][x] - v[2][x];
	v2[y] = v[1][y] - v[2][y];
	v2[z] = v[1][z] - v[2][z];

	v3[x] = v1[y] * v2[z] - v1[z] * v2[y];
	v3[y] = v1[z] * v2[x] - v1[x] * v2[z];
	v3[z] = v1[x] * v2[y] - v1[y] * v2[x];

	normalizarVetor(v3, 3, out);
}

void movement()
{
	float xN, zN;

	if (escKey)
	{
		return;
	}

	if (wKey)
	{
		xPos += versorVisionX * speed;
		yPos += versorVisionY * speed;
		zPos += versorVisionZ * speed;
		lookingAtX += versorVisionX * speed;
		lookingAtY += versorVisionY * speed;
		lookingAtZ += versorVisionZ * speed;
	}

	if (sKey)
	{
		xPos -= versorVisionX * speed;
		yPos -= versorVisionY * speed;
		zPos -= versorVisionZ * speed;
		lookingAtX -= versorVisionX * speed;
		lookingAtY -= versorVisionY * speed;
		lookingAtZ -= versorVisionZ * speed;
	}

	if (aKey)
	{
		normalize2d(versorVisionX, versorVisionZ, &xN, &zN);

		xPos += (xN * cos(90 * M_PI / 180) + zN * sin(90 * M_PI / 180)) * speed;
		lookingAtX += (xN * cos(90 * M_PI / 180) + zN * sin(90 * M_PI / 180)) * speed;

		zPos += (-xN * sin(90 * M_PI / 180) + zN * cos(90 * M_PI / 180)) * speed;
		lookingAtZ += (-xN * sin(90 * M_PI / 180) + zN * cos(90 * M_PI / 180)) * speed;
	}

	if (dKey)
	{
		normalize2d(versorVisionX, versorVisionZ, &xN, &zN);

		xPos -= (xN * cos(90 * M_PI / 180) + zN * sin(90 * M_PI / 180)) * speed;
		lookingAtX -= (xN * cos(90 * M_PI / 180) + zN * sin(90 * M_PI / 180)) * speed;

		zPos -= (-xN * sin(90 * M_PI / 180) + zN * cos(90 * M_PI / 180)) * speed;
		lookingAtZ -= (-xN * sin(90 * M_PI / 180) + zN * cos(90 * M_PI / 180)) * speed;
	}

	if (spaceKey)
	{
		yPos += speed;
		lookingAtY += speed;
	}

	if (eKey)
	{
		yPos -= speed;
		lookingAtY -= speed;
	}

	if (leftKey)
	{
		angleY -= 0.15 * speed;
	}

	if (rightKey)
	{
		angleY += 0.15 * speed;
	}

	if (upKey)
	{
		angleX -= 0.15 * speed;
	}

	if (downKey)
	{
		angleX += 0.15 * speed;
	}

	if (pgUpKey)
	{
		angleZ += 0.15 * speed;
	}

	if (pgDnKey)
	{
		angleZ -= 0.15 * speed;
	}
}

float fps()
{
	time = glutGet(GLUT_ELAPSED_TIME);

	if (time - time1 > 1000)
	{
		framerate = frame * 1000.0 / (time - time1);
		time1 = time;
		frame = 0;
	}

	return framerate;
}

float ftime()
{
	time = glutGet(GLUT_ELAPSED_TIME);
	frametime = time - lasttime;
	lasttime = time;
	return frametime;
}

void definirTitle()
{
	snprintf(
		title,
		sizeof title,
		" OpenGL-PUCPR - Formas geométricas - Caio Santos | versão %s %dx%d  %.1fFPS %.0fms ",
		ver,
		glutGet(GLUT_WINDOW_WIDTH),
		glutGet(GLUT_WINDOW_HEIGHT),
		calculatedFramerate,
		calculatedFrametime
	);

	glutSetWindowTitle(title);
}

void loadWorldOrthoProj()
{
	float w = glutGet(GLUT_WINDOW_WIDTH);
	float h = glutGet(GLUT_WINDOW_HEIGHT);

	if (h == 0)
	{
		h = 1;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (w <= h)
	{
		glOrtho(-nRange, nRange, -nRange * h / w, nRange * h / w, -zFar, zFar);
	}
	else
	{
		glOrtho(-nRange * w / h, nRange * w / h, -nRange, nRange, -zFar, zFar);
	}
}

void loadWorldPerspProj()
{
	float w = glutGet(GLUT_WINDOW_WIDTH);
	float h = glutGet(GLUT_WINDOW_HEIGHT);
	fAspect = w / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(angleV, fAspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void plano(float y, float size, int divisoes)
{
	float xmin = -size;
	float xmax = size;
	float zmin = -size;
	float zmax = size;
	float passoX = (xmax - xmin) / divisoes;
	float passoZ = (zmax - zmin) / divisoes;

	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);

	for (int i = 0; i < divisoes; i++)
	{
		for (int j = 0; j < divisoes; j++)
		{
			glVertex3f(xmin + passoX * i, y, zmin + passoZ * j);
			glVertex3f(xmin + passoX * i, y, zmin + passoZ * (j + 1));
			glVertex3f(xmin + passoX * (i + 1), y, zmin + passoZ * (j + 1));
			glVertex3f(xmin + passoX * (i + 1), y, zmin + passoZ * j);
		}
	}
	glEnd();
}

void reta(float xi, float yi, float zi, float xf, float yf, float zf)
{
	glBegin(GL_LINES);
	glVertex3f(xi, yi, zi);
	glVertex3f(xf, yf, zf);
	glEnd();
}

void cubo(float a)
{
	//glColor3f(.5, 0.2, 0.1);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(-a / 2, a / 2, a / 2);
	glVertex3f(-a / 2, -a / 2, a / 2);
	glVertex3f(a / 2, a / 2, a / 2);
	glVertex3f(a / 2, -a / 2, a / 2);
	glVertex3f(a / 2, a / 2, -a / 2);
	glVertex3f(a / 2, -a / 2, -a / 2);
	glVertex3f(-a / 2, a / 2, -a / 2);
	glVertex3f(-a / 2, -a / 2, -a / 2);
	glVertex3f(-a / 2, a / 2, a / 2);
	glVertex3f(-a / 2, -a / 2, a / 2);
	glEnd();

	//glColor3f(0.4, 0.1, 0.1);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(-a / 2, a / 2, -a / 2);
	glVertex3f(-a / 2, a / 2, a / 2);
	glVertex3f(a / 2, a / 2, -a / 2);
	glVertex3f(a / 2, a / 2, a / 2);
	glEnd();

	//glColor3f(0.4, 0.2, 0.1);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(a / 2, -a / 2, -a / 2);
	glVertex3f(a / 2, -a / 2, a / 2);
	glVertex3f(-a / 2, -a / 2, -a / 2);
	glVertex3f(-a / 2, -a / 2, a / 2);
	glEnd();
}

void cone(float radius, float height, int nLados, int divisoes)
{
	if (!divisoes) return;

	double passoRadius = radius / divisoes;
	double passoHeight = height / divisoes;

	double x, y, z, x1, y1, z1;
	float normal[3];
	bool firstLoop = true;

	glBegin(GL_TRIANGLE_FAN); //BASE
	glNormal3f(0, -1, 0);
	glVertex3f(0.0, 0.0, 0.0);
	for (double angle = (2.0 * M_PI); angle >= 0.0; angle -= (2.0 * M_PI / nLados))
	{
		x = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(x, 0.0, z);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN); // LATERAL - BICO
	float lastX = 0, lastZ = 0;

	for (double angle = 0.0; angle <= (2.0 * M_PI); angle += (2.0 * M_PI / nLados))
	{
		/*		1
				o (0,height,0)
				|\
			  | | \
			  v |  \    antihorário
				|   \
	last(x,y,z) o----o (x,y,z)
				2 ->  3
		*/

		x = passoRadius * sin(angle);
		y = height - passoHeight;
		z = passoRadius * cos(angle);

		if (firstLoop)
		{
			firstLoop = false;
			lastX = x; lastZ = z;
			continue;
		}

		float v[3][3] = {
			{ lastX, y, lastZ },
			{ x, y, z },
			{ 0, height, 0 }
		};

		calcNormal(v, normal);
		glNormal3fv(normal);

		glVertex3f(0.0, height, 0.0);
		glVertex3f(lastX, y, lastZ);
		glVertex3f(x, y, z);

		lastX = x; lastZ = z;
	}
	glEnd();

	glBegin(GL_QUADS); // LATERAL - RESTO
	double
		xT, yT, zT,
		xB, yB, zB,
		lxT = 0,
		lyT = 0,
		lzT = 0,
		lxB = 0,
		lyB = 0,
		lzB = 0;

	for (size_t i = 1; i < divisoes; i++)
	{
		firstLoop = true;

		for (double angle = (2.0 * M_PI); angle >= 0; angle -= (2.0 * M_PI / nLados))
		{
			/* 1    3
			   o----o (x,y,z)T
			   |    |
			 v |    | ^
			   |    |
			   o----o (x,y,z)B
			   2  > 4
			*/

			xT = passoRadius * i * sin(angle);
			yT = height - (passoHeight * i);
			zT = passoRadius * i * cos(angle);
			xB = passoRadius * (i + 1) * sin(angle);
			yB = height - passoHeight * (i + 1);
			zB = passoRadius * (i + 1) * cos(angle);

			if (firstLoop)
			{
				firstLoop = false;
				lxT = xT;
				lyT = yT;
				lzT = zT;
				lxB = xB;
				lyB = yB;
				lzB = zB;
				continue;
			}

			float v[3][3] = {
				{ xT, yT, zT },
				{ lxB, lyB, lzB },
				{ lxT, lyT, lzT },
			};

			calcNormal(v, normal);
			glNormal3fv(normal);

			glVertex3f(xT, yT, zT);
			glVertex3f(xB, yB, zB);
			glVertex3f(lxB, lyB, lzB);
			glVertex3f(lxT, lyT, lzT);

			lxT = xT;
			lyT = yT;
			lzT = zT;
			lxB = xB;
			lyB = yB;
			lzB = zB;
		}
	}

	glEnd();
}

void cilindro(float radius, float height, int nLados)
{
	double x, z;
	//glColor3f(0.4, 0.9, 0);
	glBegin(GL_TRIANGLE_FAN); // BASE
	glVertex3f(0.0, 0.0, 0.0); // centro
	for (double angle = (2.0 * M_PI); angle >= 0.0; angle -= (2.0 * M_PI / nLados))
	{
		x = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(x, 0.0, z);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN); //TOPO
	glVertex3f(0.0, height, 0.0); // centro
	for (double angle = 0.0; angle <= (2.0 * M_PI); angle += (2.0 * M_PI / nLados))
	{
		x = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(x, height, z);
	}
	glEnd();

	//glColor3f(0.2, 0.2, 0.7);
	glBegin(GL_TRIANGLE_STRIP); // LATERAL
	for (double angle = (2.0 * M_PI); angle >= 0.0; angle -= (2.0 * M_PI / nLados))
	{
		x = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(x, 0.0, z);
		glVertex3f(x, height, z);
	}
	glEnd();
}

void tube(float innerRadius, float height, float thickness, int nLados)
{
	float outerRadius = innerRadius + thickness;
	double x, z, xi, zi, xo, zo;

	glBegin(GL_TRIANGLE_STRIP); //BASE
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	for (double angle = 0.0; angle <= (2.0 * M_PI); angle += (2.0 * M_PI / nLados))
	{
		xi = innerRadius * sin(angle);
		zi = innerRadius * cos(angle);
		xo = outerRadius * sin(angle);
		zo = outerRadius * cos(angle);
		glVertex3f(xi, 0, zi);
		glVertex3f(xo, 0, zo);
	}
	glEnd();

	glBegin(GL_TRIANGLE_STRIP); //TOPO
	glColor3f(0.0, 1.0, 0.5);
	glVertex3f(0.0, height, 0.0);
	for (double angle = (2.0 * M_PI); angle >= 0.0; angle -= (2.0 * M_PI / nLados))
	{
		xi = innerRadius * sin(angle);
		zi = innerRadius * cos(angle);
		xo = outerRadius * sin(angle);
		zo = outerRadius * cos(angle);
		glVertex3f(xi, height, zi);
		glVertex3f(xo, height, zo);
	}
	glEnd();


	glBegin(GL_TRIANGLE_STRIP); // LATERAL INTERIOR
	glColor3f(1.0, 0.6, 0.2);
	for (double angle = (2.0 * M_PI); angle >= 0.0; angle -= (2.0 * M_PI / nLados))
	{
		x = innerRadius * sin(angle);
		z = innerRadius * cos(angle);
		glVertex3f(x, height, z);
		glVertex3f(x, 0.0, z);

	}
	glEnd();

	glBegin(GL_TRIANGLE_STRIP); // LATERAL EXTERIOR
	glColor3f(0.7, 0.4, 0.1);
	for (double angle = (2.0 * M_PI); angle >= 0.0; angle -= (2.0 * M_PI / nLados))
	{
		x = outerRadius * sin(angle);
		z = outerRadius * cos(angle);
		glVertex3f(x, 0.0, z);
		glVertex3f(x, height, z);
	}
	glEnd();
}

void comboTubes(float innerRadius, float height, float thickness, int nLados)
{
	glTranslatef(0.0, -height / 2, 0.0);
	tube(innerRadius, height, thickness, nLados);
	glTranslatef(0.0, height / 2, 0.0);

	glTranslatef(0.0, 0.0, -height / 2);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	tube(innerRadius, height, thickness, nLados);
	glRotatef(-90, 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0, 0.0, height / 2);

	glTranslatef(height / 2, 0.0, 0.0);
	glRotatef(90, 0.0f, 0.0f, 1.0f);
	tube(innerRadius, height, thickness, nLados);
	glRotatef(-90, 0.0f, 0.0f, 1.0f);
	glTranslatef(-height / 2, 0.0, 0.0);
}

void tetraHedro()
{
	float normal[3];
	glBegin(GL_TRIANGLES);
	{
		float v[3][3] = { { 0.0f, 25.0f,  0.0f }, // base em XY (face 1)
						{  25.0f, -25.0f, 0.0f },
						{ -25.0f, -25.0f, 0.0f } };
		calcNormal(v, normal);
		glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
	}
	{
		float v[3][3] = { { -25.0f, -25.0f, 0.0f}, // face 2
						{    25.0f, -25.0f, 0.0f},
						{    0.0f, 0.0f,   50.0f} };
		calcNormal(v, normal);
		glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
	}
	{
		float v[3][3] = { { 25.0f, -25.0f, 0.0f }, // face 3
						 {   0.0f,  25.0f, 0.0f },
						 {   0.0f, 0.0f,  50.0f } };
		calcNormal(v, normal);
		glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
	}
	{
		float v[3][3] = { { 0.0f, 25.0f, 0.0f }, // face 4
						 {-25.0f, -25.0f, 0.0f },
						 {  0.0f, 0.0f, 50.0f } };
		calcNormal(v, normal);
		glNormal3fv(normal);
		glVertex3fv(v[0]);
		glVertex3fv(v[1]);
		glVertex3fv(v[2]);
	}
	glEnd();
}

void draw2dBox(int bx, int by, int ux, int uy)
{
	glBegin(GL_LINE_LOOP);
	glVertex2i(bx, by);
	glVertex2i(ux, by);
	glVertex2i(ux, uy);
	glVertex2i(bx, uy);
	glEnd();
}

void draw2dBoxFilled(int bx, int by, int ux, int uy)
{
	glBegin(GL_POLYGON);
	glVertex2i(bx, by);
	glVertex2i(ux, by);
	glVertex2i(ux, uy);
	glVertex2i(bx, uy);
	glEnd();
}

void xyzLines()
{
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); // Red - X
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(20.0, 0.0, 0.0);
	glColor3f(0.0, 1.0, 0.0); // Green - Y
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 20.0, 0.0);
	glColor3f(0.0, 0.0, 1.0); // Blue - Z
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 20.0);
	glEnd();
}

void xyzLines3d(float sizeFactor = 1, float lengthFactor = 1, float thicknessFactor = 1, bool pointy = 0)
{
	float finalLength = sizeFactor * lengthFactor;
	float finalThickness = sizeFactor * thicknessFactor;

	glColor3f(1.0, 0.0, 0.0); // Red - X 
	glLoadName(1);
	glRotatef(-90, 0.0f, 0.0f, 1.0f);
	cilindro(0.02 * finalThickness, 20 * finalLength, 10);
	glTranslatef(0, 20 * finalLength, 0);
	cone(0.1 * finalThickness, 0.4 * finalLength + (pointy * 10 * sizeFactor), 10, 1);
	glTranslatef(0, -20 * finalLength, 0);
	glRotatef(90, 0.0f, 0.0f, 1.0f);

	glColor3f(0.0, 1.0, 0.0); // Green - Y
	glLoadName(2);
	cilindro(0.02 * finalThickness, 20 * finalLength, 10);
	glTranslatef(0, 20 * finalLength, 0);
	cone(0.1 * finalThickness, 0.4 * finalLength + (pointy * 10 * sizeFactor), 10, 1);
	glTranslatef(0, -20 * finalLength, 0);

	glColor3f(0.0, 0.0, 1.0); // Blue - Z 
	glLoadName(3);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	cilindro(0.02 * finalThickness, 20 * finalLength, 10);
	glTranslatef(0, 20 * finalLength, 0);
	cone(0.1 * finalThickness, 0.4 * finalLength + (pointy * 10 * sizeFactor), 10, 1);
	glTranslatef(0, -20 * finalLength, 0);
	glRotatef(-90, 1.0f, 0.0f, 0.0f);
}

void rotationTorus3d(float rx, float ry, float rz, float sizeFactor = 1)
{
	glRotatef(ry, 0.0f, 1.0f, 0.0f);

	glColor3f(0.0, 1.0, 0.0); // Green - Y
	glLoadName(5);
	glPushMatrix();
	glRotatef(-90, 1.0f, 0.0f, 0.0f);
	glutSolidTorus(0.2, 10, 30, 30);
	glPopMatrix();

	glRotatef(rx, 1.0f, 0.0f, 0.0f);

	glColor3f(1.0, 0.0, 0.0); // Red - X 
	glLoadName(4);
	glPushMatrix();
	glRotatef(90, 0.0f, 1.0f, 0.0f);
	glutSolidTorus(0.2, 10, 30, 30);
	glPopMatrix();

	glRotatef(rz, 0.0f, 0.0f, 1.0f);

	glColor3f(0.0, 0.0, 1.0); // Blue - Z 
	glLoadName(6);
	glPushMatrix();
	glRotatef(0, 0.0f, 0.0f, 0.0f);
	glutSolidTorus(0.2, 10, 30, 30);
	glPopMatrix();
}

void renderCoords()
{
	glColor3f(1.0, 0.0, 0.0);
	renderString3D(20, 1.0f, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, "X");
	glColor3f(0.0, 1.0, 0.0);
	renderString3D(0.0f, 20 + 1, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, "Y");
	glColor3f(0.0, 0.0, 1.0);
	renderString3D(0.0f, 1.0f, 20, GLUT_BITMAP_TIMES_ROMAN_24, "Z");
}

void resumeButton()
{
	escKey = false;
}

void quit()
{
	glutDestroyWindow(glutGetWindow());
	exit(0);
}

void escapeMenu(int screenX, int screenY)
{
	int buttonWidth = 200;
	int buttonHeight = 40;

	menuEsc.desenharMenu(screenX, screenY, mouseX, mouseY, lClick);
}

bool textInput = false;
bool textInterface = false;
bool _adding_obj = false;
bool _editing_r = false;
bool _editing_g = false;
bool _editing_b = false;
bool enterKey = false;
int inputIndex = 0;
int _add_tipoObj = 0;
float _add_r, _add_g, _add_b;
std::vector<float> _add_parametros;

void enterInputMode()
{
	textInterface = true;
	textInput = true;
	glutSetKeyRepeat(GLUT_KEY_REPEAT_ON);
	glutKeyboardFunc(keyboardTextInput);
}

void exitInputMode()
{
	/* Se não estamos no modo textInput, o callback e o comportamento do teclado volta ao normal */
	textInput = false;
	textInterface = false;
	enterKey = false;
	inputIndex = 0;
	_add_tipoObj = 0;
	_add_parametros.clear();
	inputString = "";

	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutKeyboardFunc(processNormalKeys);
}

void keyboardTextInput(unsigned char key, int x, int y)
{
	if (textInput)
	{
		/* Estamos no modo textInput, o teclado se comporta como um editor de texto */

		if (key == '\b') /* Quando BACKSPACE é pressionado */
		{
			try
			{
				inputString.pop_back(); /* Apaga o último caractere */
			}
			catch (const std::exception&) /* Não há caractere para apagar*/
			{
				return;
			}
			return;
		}

		if (key == 27) /* Quando ESC é pressionado aborta o processo limpando todas as variaveis */
		{
			exitInputMode();
			return;
		}

		if (key == 13) /* Quando ENTER é pressionado */
		{
			cout << "Linha digitada: " << inputString << endl;
			enterKey = true;
			return; /* Sai da função, não adicionando a tecla enter à string de input */
		}

		inputString += key; // A tecla digitada é adicionada ao inputString para ser exibida na tela e/ou processada.
	}
	else
	{
		exitInputMode();
	}
}

float processarCor(float input)
{
	if (input <= 0) return 0;
	else if (input <= 1) return input;
	else if (input <= 255) return input / 255;
	else return 1;
}

void _addObj()
{
	_adding_obj = true;

	float w = glutGet(GLUT_WINDOW_WIDTH);
	float h = glutGet(GLUT_WINDOW_HEIGHT);

	std::vector<string> nomeParametros;

	enterInputMode();

	dialogString = "Digite o tipo de objeto (1-10): ";

	if (inputIndex > 0)
	{
		switch (_add_tipoObj)
		{
		case 0: // reta3d
			nomeParametros = { "xi", "yi", "zi", "xf", "yf", "zf" };
			break;
		case 1: // cubo
			nomeParametros = { "aresta (float)" };
			break;
		case 2: // cone
			nomeParametros = { "raio (float)","altura (float)","nLados (int)", "divisoes (int)" };
			break;
		case 3: // cilindro
			nomeParametros = { "raio (double)", "altura (double)", "slices (int)", "stacks (int)" };
			break;
		case 4: // tubo
			nomeParametros = { "raio interior (float)", "altura (float)", "espessura (float)", "nLados (int)" };
			break;
		case 5: // combo tubos
			nomeParametros = { "raio interior (float)", "altura (float)", "espessura (float)", "nLados (int)" };
			break;
		case 6: // glut teapot
			nomeParametros = { "tamanho (float)" };
			break;
		case 7: // glut cubo
			nomeParametros = { "tamanho (float)" };
			break;
		case 8: // glut sphere
			nomeParametros = { "raio (float)", "divisoes hori. (int)", "divisoes vert. (int)" };
			break;
		case 9: // glut teacup
			nomeParametros = { "tamanho (float)" };
			break;
		case 10: // glut teaspoon
			nomeParametros = { "tamanho (float)" };
			break;
		case 11: // plano
			nomeParametros = { "y (float)", "tamanho (float)", "divisoes (int)" };
			break;
		default:
			break;
		}
	}
	try
	{
		if (inputIndex == 0)
		{
			dialogString = "Digite o tipo de objeto (1-10): ";
			if (enterKey)
			{
				_add_tipoObj = std::stoi(inputString);
				inputString = ""; // Apaga string que o usuário escreveu
				enterKey = false;
				inputIndex++; // Adiciona 1 ao indice de input
				if (_add_tipoObj < 0 || _add_tipoObj > 11)
				{
					inputIndex = 1000;
				}
			}
		}
		else if (inputIndex == 1)
		{
			dialogString = "Digite a cor Red (float 0-1)(int 0-255): ";
			if (enterKey)
			{
				_add_r = processarCor(std::stof(inputString));
				inputString = ""; // Apaga string que o usuário escreveu
				enterKey = false;
				inputIndex++; // Adiciona 1 ao indice de input
			}
		}
		else if (inputIndex == 2)
		{
			dialogString = "Digite a cor Green (float 0-1)(int 0-255): ";
			if (enterKey)
			{
				_add_g = processarCor(std::stof(inputString));
				inputString = ""; // Apaga string que o usuário escreveu
				enterKey = false;
				inputIndex++; // Adiciona 1 ao indice de input
			}
		}
		else if (inputIndex == 3)
		{
			dialogString = "Digite a cor Blue (float 0-1) (int 0-255): ";
			if (enterKey)
			{
				_add_b = processarCor(std::stof(inputString));
				inputString = ""; // Apaga string que o usuário escreveu
				enterKey = false;
				inputIndex++; // Adiciona 1 ao indice de input
			}
		}
		else if ((inputIndex - 3) <= nomeParametros.size())
		{
			dialogString = "Digite o parametro: " + nomeParametros[inputIndex - 4];
			if (enterKey)
			{
				_add_parametros.push_back(std::stof(inputString));
				inputString = ""; // Apaga string que o usuário escreveu
				enterKey = false;
				inputIndex++; // Adiciona 1 ao indice de input
			}
		}
		else if (inputIndex == 1000)
		{
			dialogString = "Parâmetro inválido recebido. Reinicie o processo. (ESC)";
			if (enterKey)
			{
				_add_parametros.push_back(std::stof(inputString));
				inputString = "INVÁLIDO"; // Apaga string que o usuário escreveu
				enterKey = false;
			}
		}
		else
		{
			// objeto criado e adicionado na origem do sis. coordenadas
			ObjetoOpenGL novoObj(_add_tipoObj, 0.0, 0.0, 0.0, 0.0f, 0.0f, 0.0f, _add_r, _add_g, _add_b, _add_parametros);
			Objetos[forma - 1].partes.emplace_back(novoObj);
			_adding_obj = false;
			exitInputMode();
		}
	}
	catch (const std::exception&)
	{
		inputIndex = 1000;
	}
}

void _resetX()
{
	Objetos[forma - 1].partes[parteIdx].x = 0;
}

void _resetY()
{
	Objetos[forma - 1].partes[parteIdx].y = 0;
}

void _resetZ()
{
	Objetos[forma - 1].partes[parteIdx].z = 0;
}

void _editR() //--------------------------
{
	_editing_r = true;
	enterInputMode();
	dialogString = "Digite a cor Red (float 0-1)(int 0-255): ";

	if (inputIndex == 0)
	{
		if (enterKey)
		{
			_add_r = processarCor(std::stof(inputString));
			inputString = ""; // Apaga string que o usuário escreveu
			enterKey = false;
			inputIndex++; // Adiciona 1 ao indice de input
		}
	}
	else
	{
		Objetos[forma - 1].partes[parteIdx].r = _add_r; /* Modifica a cor editando o objeto diretamente */
		_editing_r = false;
		exitInputMode();
	}
}

void _editG()
{
	_editing_g = true;
	enterInputMode();
	dialogString = "Digite a cor Green (float 0-1)(int 0-255): ";

	if (inputIndex == 0)
	{
		if (enterKey)
		{
			_add_g = processarCor(std::stof(inputString));
			inputString = ""; // Apaga string que o usuário escreveu
			enterKey = false;
			inputIndex++; // Adiciona 1 ao indice de input
		}
	}
	else
	{
		Objetos[forma - 1].partes[parteIdx].g = _add_g; /* Modifica a cor editando o objeto diretamente */
		_editing_g = false;
		exitInputMode();
	}
}

void _editB()
{
	_editing_b = true;
	enterInputMode();
	dialogString = "Digite a cor Blue (float 0-1)(int 0-255): ";

	if (inputIndex == 0)
	{
		if (enterKey)
		{
			_add_b = processarCor(std::stof(inputString));
			inputString = ""; // Apaga string que o usuário escreveu
			enterKey = false;
			inputIndex++; // Adiciona 1 ao indice de input
		}
	}
	else
	{
		Objetos[forma - 1].partes[parteIdx].b = _add_b; /* Modifica a cor editando o objeto diretamente */
		_editing_b = false;
		exitInputMode();
	}
}

void _resetRx()
{
	Objetos[forma - 1].partes[parteIdx].rX = 0;
}

void _resetRy()
{
	Objetos[forma - 1].partes[parteIdx].rY = 0;
}

void _resetRz()
{
	Objetos[forma - 1].partes[parteIdx].rZ = 0;
}

void _remove()
{
	Objetos[forma - 1].partes.erase(Objetos[forma - 1].partes.begin() + parteIdx); // Remove ObjetoOpenGL do array partes do objeto composto
	parteSelecionada = 0; // Desseleciona objeto
	lClick = false; // Garante que apenas um objeto é deletado por click
}

void fecharInstrucoes()
{
	slashKey = !slashKey;
}

void renderInterface()
{
	float w = glutGet(GLUT_WINDOW_WIDTH);
	float h = glutGet(GLUT_WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION); // setting up an orthogonal projection proportional to screen resolution
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1000, 1000);
	glMatrixMode(GL_MODELVIEW); // back to model view
	glLoadIdentity();

	glColor3f(1.0, 0.6, 0.0);
	char mouseBuffer[50]; // mouse-cursor-following-text buffer

	snprintf(
		mouseBuffer,
		sizeof mouseBuffer,
		"(%d, %d)\nYaw = %.1f\nPitch = %.1f",
		mouseX,
		mouseY,
		cameraYaw,
		cameraPitch);

	renderString(mouseX - 5 + 7, h - mouseY - 4 - 14, GLUT_BITMAP_8_BY_13, mouseBuffer); // coords, pitch, yaw aparecem abaixo do cursor

	if (escKey) // se ESC foi pressionado, apenas o menu é desenhado e o movimento é bloqueado
	{
		escapeMenu(w, h);
		return;
	}

	char buffer[1024];
	snprintf(buffer, sizeof buffer,
		"ProjectionMode = %s    AspectRatio= %f    Resolution= %.0fx%.0f    FPS= %.1f    frametime= %3.0fms   speed= %.3f\n"
		"%s= %.1f%s\n"
		"X= %.1f°\n"
		"Y= %.1f°\n"
		"Z= %.1f°\n"
		"(r)eset\n"
		"X (up)-(down)\n"
		"Y (left)-(right)\n"
		"Z (pgup)-(pgdn)\n\n"
		"a(n)imate= %s\n"
		"animate(x)= %s\n"
		"animate(y)= %s\n"
		"animate(z)= %s\n\n"
		"GL_(F)RONT = %s\n"
		"GL_(B)ACK = %s\n\n"
		"GL_(C)ULL_FACE = %s\n"
		"GL_DEPTH_(T)EST = %s\n"
		"glShadeMode(L) = %s\n\n"
		"xPos= %f\n"
		"yPos= %f\n"
		"zPos= %f\n"
		"lookingAtX= %f\n"
		"lookingAtY= %f\n"
		"lookingAtZ= %f\n"
		"visionX= %f\n"
		"visionY= %f\n"
		"visionZ= %f\n",
		projMode ? "glOrtho" : "gluPerspective",
		(w / h), w, h, calculatedFramerate, calculatedFrametime, speed,
		projMode ? "nRange" : "angleV",
		projMode ? nRange : angleV,
		projMode ? "" : "°",
		rotX, rotY, rotZ,
		animate ? "true" : "false",
		animateX ? "true" : "false",
		animateY ? "true" : "false",
		animateZ ? "true" : "false",
		front ? "GL_LINE" : "GL_FILL",
		back ? "GL_LINE" : "GL_FILL",
		cface ? "enable" : "disable",
		depthTest ? "enable" : "disable",
		shadeModel ? "GL_SMOOTH" : "GL_FLAT",
		xPos, yPos, zPos,
		lookingAtX, lookingAtY, lookingAtZ,
		visionX, visionY, visionZ
	);

	glColor3f(1.0, 1.0, 0.0);
	renderString(5, h - 29, GLUT_BITMAP_9_BY_15, buffer);

	snprintf(buffer, sizeof buffer,
		"GL_MODELVIEW_MATRIX \n"
		"| % -3.3f | % -3.3f | % -3.3f | % -3.3f | \n"
		"| % -3.3f | % -3.3f | % -3.3f | % -3.3f | \n"
		"| % -3.3f | % -3.3f | % -3.3f | % -3.3f | \n"
		"| % -6.1f | % -6.1f | % -6.1f | % -6.1f | \n",
		Mmodelview[0], Mmodelview[1], Mmodelview[2], Mmodelview[3],
		Mmodelview[4], Mmodelview[5], Mmodelview[6], Mmodelview[7],
		Mmodelview[8], Mmodelview[9], Mmodelview[10], Mmodelview[11],
		Mmodelview[12], Mmodelview[13], Mmodelview[14], Mmodelview[15]
	);

	renderString(w - 350, h - 59, GLUT_BITMAP_9_BY_15, buffer);

	glPushMatrix(); // Mini camera XYZ axis
	glTranslatef(w - 70, 70, 0); // Posicionamento
	glRotatef(-cameraPitch, 1.0f, 0.0f, 0.0f);// Camera rotation 
	glRotatef(cameraYaw + 90, 0.0f, 1.0f, 0.0f);
	xyzLines3d(.5, 5, 100, 1);
	glPopMatrix();

	char menuBuffer[1024] = "";
	char itemBuffer[100];
	if (mKey)
	{
		for (size_t i = 0; i < Objetos.size(); i++)
		{
			snprintf(itemBuffer, sizeof itemBuffer,
				"(%d) %s %s %s \n",
				i + 1,
				forma == i + 1 ? "[ " : " ",
				Objetos[i].nome,
				forma == i + 1 ? "] " : " "
			);
			strcat_s(menuBuffer, sizeof(menuBuffer), itemBuffer);
		}
	}
	strcat_s(menuBuffer, sizeof(menuBuffer), "Pressione M para menu de objetos");
	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0);
	renderString(5, 20 + 16 * Objetos.size() * mKey, GLUT_BITMAP_9_BY_15, menuBuffer);
	glPopMatrix();

	char instructionsBuffer[4096] = "     Pressione '/' para exibir instruções\n\n";
	if (slashKey)
	{
		glPushMatrix();
		glColor3f(0.1, 0.1, 0.1);
		draw2dBoxFilled(490, h - 40, 1150, h - 550); // Desenha caixa de informações
		glColor3f(1.0, 0.8, 0.0);
		draw2dBox(490, h - 40, 1150, h - 550);
		draw2dBox(490, h - 70, 1150, h - 70);
		glPopMatrix();

		Botao fecharInstrucoes("Fechar", 70, 15, fecharInstrucoes);
		fecharInstrucoes.desenharBotao(1110, h - 55, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);

		strcat_s(instructionsBuffer, sizeof(instructionsBuffer),
			"ESC      -> Menu de opções \n"
			"1 - 10   -> Selecionar cena  \n"
			"w        -> Movimento para frente \n"
			"a        -> Movimento para esquerda \n"
			"s        -> Movimento para trás \n"
			"d        -> Movimento para direita\n"
			"e        -> Movimento para baixo \n"
			"Spacebar -> Movimento para cima \n"
			"Setas    -> Rotação global \n"
			"PgUp     -> Rotação global \n"
			"PgDn     -> Rotação global \n"
			"i        -> Iluminação \n"
			"L        -> Toggle ShadeMode (SMOOTH ou FLAT) \n"
			"r        -> Reset \n"
			"n        -> Animação automática (ainda é necessário ativar cada eixo)\n"
			"x, y, z  -> Animação automática em cada eixo \n"
			"f        -> Toggle GL_FRONT (FILL ou LINE) \n"
			"b        -> Toggle GL_BACK (FILL ou LINE) \n"
			"c        -> Toggle CULL_FACE \n"
			"t        -> Toggle DEPTH_TEST \n"
			"v        -> Toggle velocidade \n"
			"p        -> Toggle perspectiva (Projeção ou Ortogonal) \n"
			"m        -> Menu de objetos compostos (displayfile) \n"
			"F1       -> Carregar displayfile \n"
			"F2       -> Salvar modificações no displayfile \n"
			"F11      -> Toggle FULLSCREEN \n"
			"\n"
			"Mouse esquerdo           -> Seleciona objeto e interage com a interface \n"
			"Mouse direito + arrastar -> Movimento da câmera \n"
		);
	}

	glPushMatrix();
	glColor3f(1.0, 1.0, 0.0);
	renderString(500, h - 60, GLUT_BITMAP_9_BY_15, instructionsBuffer);
	glPopMatrix();

	if (forma == 5)
	{
		gluProject(150.0, 18.0, 0.0, Mmodelview, Mprojection, viewport, &winX, &winY, &winZ);
		if (winZ < 1)
		{
			glPushMatrix();
			glBegin(GL_LINES);
			glVertex2f(w / 2, 0);
			glVertex2f(winX, winY);
			glEnd();
			renderString(winX + 2, winY + 9, GLUT_BITMAP_9_BY_15, "Terra");
			glPopMatrix();
		}
	}
	if (forma == 10)
	{
		glPushMatrix();
		glColor3f(0.1, 0.1, 0.1);
		draw2dBoxFilled(240, 80, 1250, 48); // Desenha caixa de informações
		glColor3f(1.0, 0.8, 0.0);
		draw2dBox(240, 80, 1250, 48);
		glColor3f(1.0, 1.0, 0.0);
		renderString(250, 60, GLUT_BITMAP_9_BY_15, "Neste objeto a iluminação foi desativada e o glPolygonMode(GL_FRONT) está em GL_LINES para melhor visualização");
		glPopMatrix();
	}

	Botao addObj("Adicionar Objeto", 250, 40, _addObj);
	addObj.desenharBotao(300, h - 200, mouseX, h - mouseY, lClick, true, GLUT_STROKE_ROMAN);

	if (parteSelecionada)
	{
		glColor3f(1, 1, 1);
		gluProject(objSelecionado.x, objSelecionado.y, objSelecionado.z, Mmodelview, Mprojection, viewport, &winX, &winY, &winZ);
		if (winZ < 1) // Desenha linha saindo ddo centro da caixa até o centro do objeto
		{
			glPushMatrix();
			glBegin(GL_LINES);
			glVertex3f(winX, winY, winZ);
			glVertex3f(322, (h - 490), 0);
			glEnd();
			glPopMatrix();
		}
		glColor3f(0.02, 0.02, 0.02);
		draw2dBoxFilled(245, h - 380, 410, h - 600); // Desenha caixa para as informações do objeto
		glColor3f(1.0, 0.8, 0.0);
		draw2dBox(245, h - 380, 410, h - 600); // Desenha caixa para as informações do objeto

		//Botao prevTipo("<", 18, 13, _prevTipo);
		//prevTipo.desenharBotao(370, h - 397, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		//Botao nextTipo(">", 18, 13, _nextTipo);
		//nextTipo.desenharBotao(394, h - 397, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao resetX("Reset", 50, 13, _resetX);
		resetX.desenharBotao(380, h - 413, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao resetY("Reset", 50, 13, _resetY);
		resetY.desenharBotao(380, h - 429, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao resetZ("Reset", 50, 13, _resetZ);
		resetZ.desenharBotao(380, h - 445, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao resetR("Edit", 50, 13, _editR);
		resetR.desenharBotao(380, h - 461, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao resetG("Edit", 50, 13, _editG);
		resetG.desenharBotao(380, h - 477, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao resetB("Edit", 50, 13, _editB);
		resetB.desenharBotao(380, h - 493, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao resetRx("Reset", 50, 13, _resetRx);
		resetRx.desenharBotao(380, h - 509, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao resetRy("Reset", 50, 13, _resetRy);
		resetRy.desenharBotao(380, h - 525, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao resetRz("Reset", 50, 13, _resetRz);
		resetRz.desenharBotao(380, h - 541, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);
		Botao remove("Remover", 155, 15, _remove);
		remove.desenharBotao(329, h - 557, mouseX, h - mouseY, lClick, false, GLUT_BITMAP_8_BY_13);

		snprintf(buffer, sizeof buffer,
			"tipo = %d\n"
			"x = %.1f\n"
			"y = %.1f\n"
			"z = %.1f\n"
			"r = %.4f\n"
			"g = %.4f\n"
			"b = %.4f\n"
			"rx = %.1f°\n"
			"ry = %.1f°\n"
			"rz = %.1f°\n",
			objSelecionado.tipo,
			objSelecionado.x,
			objSelecionado.y,
			objSelecionado.z,
			objSelecionado.r,
			objSelecionado.g,
			objSelecionado.b,
			objSelecionado.rX,
			objSelecionado.rY,
			objSelecionado.rZ);

		glColor3f(1.0, 1.0, 1.0);
		// Renderiza texto branco dentro da caixa de informações do objeto.
		renderString(250, h - 401, GLUT_BITMAP_9_BY_15, buffer);
	}

	if (textInterface)
	{
		if (_adding_obj)
		{
			_addObj();
		}
		else if (_editing_r)
		{
			_editR();
		}
		else if (_editing_g)
		{
			_editG();
		}
		else if (_editing_b)
		{
			_editB();
		}
		glColor3f(0.1, 0.1, 0.1);
		draw2dBoxFilled(180, h - 50, 700, h - 150); // Desenha caixa para as informações do objeto
		glColor3f(1.0, 0.8, 0.0);
		draw2dBox(180, h - 50, 700, h - 150); // Desenha caixa para as informações do objeto
		draw2dBox(180, h - 70, 700, h - 70);
		renderString(185, h - 65, GLUT_BITMAP_9_BY_15, dialogString.c_str());
		renderString(185, h - 84, GLUT_BITMAP_9_BY_15, inputString.c_str());
	}
}

void renderWorld()
{
	int nome, i = 0;
	nome = 7;
	gluLookAt(xPos, yPos, zPos, lookingAtX, lookingAtY, lookingAtZ, 0, 1, 0);

	front ? glPolygonMode(GL_FRONT, GL_LINE) : glPolygonMode(GL_FRONT, GL_FILL);
	back ? glPolygonMode(GL_BACK, GL_LINE) : glPolygonMode(GL_BACK, GL_FILL);

	movement(); // movimento WASD
	moving = false;
	if (wKey || aKey || sKey || dKey || spaceKey || eKey)
	{
		moving = true;
	}

	angleX = (angleX >= 360) ? (angleX -= 360) : (angleX < 0) ? (angleX += 360) : angleX; // 0 <= angleX < 360
	angleY = (angleY >= 360) ? (angleY -= 360) : (angleY < 0) ? (angleY += 360) : angleY;
	angleZ = (angleZ >= 360) ? (angleZ -= 360) : (angleZ < 0) ? (angleZ += 360) : angleZ;

	rotX = angleX += 0.2 * animate * animateX * speed; // rotX = ângulo modificado pelas setas * speed factor(0.2) * bool animação global * bool animação em eixo específico * modificador speed
	rotY = angleY += 0.2 * animate * animateY * speed;
	rotZ = angleZ += 0.2 * animate * animateZ * speed;

	visionMag = sqrt(pow(visionX, 2) + pow(visionY, 2) + pow(visionZ, 2));
	versorVisionX = (visionX = (lookingAtX - xPos)) / visionMag;
	versorVisionY = (visionY = (lookingAtY - yPos)) / visionMag;
	versorVisionZ = (visionZ = (lookingAtZ - zPos)) / visionMag;

	//--------------------------------------------------------------------
	GLfloat luzAmbiente[4] = { 0.05, 0.05, 0.05, 1.0 };
	GLfloat luzDifusa[4] = { 0.9, 0.9, 0.9, 1.0 };	   // "cor" 
	GLfloat luzEspecular[4] = { 1.0, 1.0, 1.0, 1.0 };// "brilho" 


	GLfloat posicaoLuz[4] = { 0.0, 18.0, 0.0, 1.0 };

	// Capacidade de brilho do material
	GLfloat especularidade[4] = { 1.0,1.0,1.0,1.0 };
	GLint especMaterial = 60;

	if (forma == 4) /* Semáforo */
	{
		luzDifusa[0] = 0.2;
		luzDifusa[1] = 1.0;
		luzDifusa[2] = 0.2;
		luzDifusa[3] = 1.0;	   // "cor" 
		luzEspecular[0] = 0.3;
		luzEspecular[1] = 1.0;
		luzEspecular[2] = 0.3;
		luzEspecular[3] = 1.0;
		posicaoLuz[0] = 1.8;
		posicaoLuz[1] = 38.0079;
		posicaoLuz[2] = 4.92311;
		posicaoLuz[3] = 1.0;
		especMaterial = 20;
	}

	// Habilita o modelo de colorização de Gouraud
	//glShadeModel(GL_SMOOTH); // glShadeModel(GL_FLAT);
	// Define a refletância do material 
	glMaterialfv(GL_FRONT, GL_SPECULAR, especularidade);
	// Define a concentração do brilho
	glMateriali(GL_FRONT, GL_SHININESS, especMaterial);
	// Ativa o uso da luz ambiente 
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);
	// Define os parâmetros da luz de número 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);
	glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz);

	//glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 2);
	//glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.005f);

	// Habilita a definição da cor do material a partir da cor corrente

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);

	glTranslatef(posicaoLuz[0], posicaoLuz[1], posicaoLuz[2]);
	glColor3f(luzDifusa[0], luzDifusa[1], luzDifusa[2]);
	glutSolidSphere((696340.0 / 1000000.0), 100, 100);
	if (forma == 4)
	{
		glutSolidSphere(1.05, 100, 100);
	}
	glTranslatef(-posicaoLuz[0], -posicaoLuz[1], -posicaoLuz[2]);

	globalIllumination ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);

	glColor3f(0.3, 0.3, 0.3);
	//plano(-20, 100, 300); // plano fixo

	glRotatef(rotX, 1.0f, 0.0f, 0.0f); // global rotation 
	glRotatef(rotY, 0.0f, 1.0f, 0.0f);
	glRotatef(rotZ, 0.0f, 0.0f, 1.0f);

	glGetDoublev(GL_MODELVIEW_MATRIX, Mmodelview);
	glGetDoublev(GL_PROJECTION_MATRIX, Mprojection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glDisable(GL_LIGHTING);
	xyzLines();
	renderCoords();

	objetoCompostoAtual = Objetos[forma - 1];

	if (!strcmp(objetoCompostoAtual.nome, "SistemaSolar"))
	{
		glColor3f(1.0, 1.0, 1.0);
		renderString3D(57.91, 18.01, 0.0, GLUT_BITMAP_9_BY_15, "Mercurio");
		renderString3D(108.2, 18.01, 0.0, GLUT_BITMAP_9_BY_15, "Venus");
		renderString3D(150, 18.01, 0.3844, GLUT_BITMAP_9_BY_15, "Lua");
		renderString3D(227.9, 18.01, 0.0, GLUT_BITMAP_9_BY_15, "Marte");
		renderString3D(778.57, 18.01, 0.0, GLUT_BITMAP_9_BY_15, "Jupiter");
	}

	for (ObjetoOpenGL r : Retas) /* Desenha as retas geradas na função desenharRaycast */
	{
		glColor3f(1, 1, 1);
		glPushMatrix();
		reta(r.params[0], r.params[1], r.params[2], r.params[3], r.params[4], r.params[5]);
		glPopMatrix();
	}

	for (ObjetoOpenGL parte : objetoCompostoAtual.partes)
	{
		globalIllumination ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
		glColor3f(parte.r, parte.g, parte.b);
		glLoadName(nome); /* A cada objeto é atribuído um nome, e este nome é adicionado a uma variável do próprio objeto*/
		parte.id = nome;  /* Então podemos realizar alguma ação sobre o objeto selecionado */

		if (parteSelecionada == parte.id)
		{
			/* Se o objeto do loop atual é o selecionado */
			glDisable(GL_LIGHTING);
			objSelecionado = parte;
			parteIdx = i;

			if (!moving && rotX == 0 && rotY == 0 && rotZ == 0)
			{
				/* 
				* Se a câmera não está se movendo, e se não tiver sido feita nenhuma rotação global (usando as setas)
				* É desenhado entre a posição da câmera e a posição do objeto um  xyzLines3d, que o usuário pode arrastar
				* para movimentar o objeto.
				* Também é desenhado um rotationTorus3d que pode ser arrastado para cima ou para baixo, rotacionando o
				* objeto em cada eixo correspondente. A ordem de rotação é Y - X - Z.
				*/
				float x = xPos - parte.x;
				float y = yPos - parte.y;
				float z = zPos - parte.z;

				float vo[3], v[3] = { x,y,z };
				normalizarVetor(v, 3, vo);

				/* v = vetor do objeto até a câmera */
				/* vo = v normalizado*/

				glPushMatrix();
				glTranslatef(xPos - vo[0] * 5, yPos - vo[1] * 5, zPos - vo[2] * 5);
				xyzLines3d(.005, 5, 100, 1);
				glPopMatrix();

				glPushMatrix();
				glTranslatef(parte.x, parte.y, parte.z);
				rotationTorus3d(parte.rX, parte.rY, parte.rZ);
				glPopMatrix();

			}
			glColor3f(0, 0.5, 0);
			glLoadName(nome); // Refaz glLoadName senão o nome é definido pelo último glLoadName dentro de xyzLines3d()
		}

		glPushMatrix();
		glTranslatef(parte.x, parte.y, parte.z);
		glRotatef(parte.rY, 0.0f, 1.0f, 0.0f);
		glRotatef(parte.rX, 1.0f, 0.0f, 0.0f);
		glRotatef(parte.rZ, 0.0f, 0.0f, 1.0f);

		switch (parte.tipo)
		{
		case 0:
			// float xi, float yi, float zi, float xf, float yf, float zf
			reta(parte.params[0], parte.params[1], parte.params[2], parte.params[3], parte.params[4], parte.params[5]);
			break;
		case 1:
			// float aresta
			cubo(parte.params[0]);
			break;
		case 2:
			// float radius, float height, int nLados, int divisoes
			cone(parte.params[0], parte.params[1], parte.params[2], parte.params[3]);
			break;
		case 3:
			// float radius, float height, int nLados
			//cilindro(parte.params[0], parte.params[1], parte.params[2]);

			// double radius, double height, GLint slices, GLint stacks
			glutSolidCylinder(parte.params[0], parte.params[1], parte.params[2], parte.params[3]);
			break;
		case 4:
			// float innerRadius, float height, float thickness, int nLados
			tube(parte.params[0], parte.params[1], parte.params[2], parte.params[3]);
			break;
		case 5:
			// float innerRadius, float height, float thickness, int nLados
			comboTubes(parte.params[0], parte.params[1], parte.params[2], parte.params[3]);
			break;
		case 6:
			// float size
			glutSolidTeapot(parte.params[0]);
			break;
		case 7:
			glutSolidCube(parte.params[0]);
			break;
		case 8:
			glutSolidSphere(parte.params[0], parte.params[1], parte.params[2]);
			break;
		case 9:
			glutSolidTeacup(parte.params[0]);
			break;
		case 10:
			glutSolidTeaspoon(parte.params[0]);
			break;
		case 11:
			glMateriali(GL_FRONT, GL_SHININESS, 5); /* especularidade do plano reduzida */
			plano(parte.params[0], parte.params[1], parte.params[2]);
			break;
		}
		glPopMatrix();
		i++;
		nome++;
	}
}

void render()
{
	frame++; // Adiciona um frame para o cálculo do frametime e framerate
	calculatedFrametime = ftime();
	calculatedFramerate = fps();
	definirTitle(); // Define o título da janela

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

	//3D
	depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST); // Prepara o mundo para renderização 3D
	projMode ? loadWorldOrthoProj() : loadWorldPerspProj(); // Carrega perspectiva escolhida
	if (!projMode)
	{
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	glMatrixMode(GL_MODELVIEW); // De volta à matriz modelview
	glLoadIdentity(); // Carrega matriz identidade
	renderWorld(); // 3d stuff

	// 2D, Interface
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	renderInterface();

	glutSwapBuffers();
	glutPostRedisplay();

	// Modificador speed é aplicado no movimento para ser o mesmo independente da taxa de quadros
	speed = calculatedFrametime * (0.05f - 0.04 * speedModifierBool);
}

void reshape(GLsizei w, GLsizei h)
{
	if (h == 0) h = 1;
	glViewport(0, 0, w, h);
}

int main(int argc, char** argv)
{
	displayFileLoad();              // se estiver aqui, le somente uma vez
	menuEsc.inicializarMenu();

	glutInit(&argc, argv);            // Initialize GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_MULTISAMPLE);

	int RESOLUTION_INITIAL_WIDTH = glutGet(GLUT_SCREEN_WIDTH) - 80;
	int RESOLUTION_INITIAL_HEIGHT = glutGet(GLUT_SCREEN_HEIGHT) - 120;

	glutInitWindowSize(RESOLUTION_INITIAL_WIDTH, RESOLUTION_INITIAL_HEIGHT);     // Set the window's initial width & height
	glutInitWindowPosition(40, 40);   // Position the window's initial top-left corner
	glutCreateWindow("");          // Create window with the given title
	glutDisplayFunc(render);          // Register callback handler for window re-paint event
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutKeyboardFunc(processNormalKeys);
	glutKeyboardUpFunc(processNormalKeysUp);
	glutSpecialFunc(processSpecialKeys);
	glutSpecialUpFunc(processSpecialKeysUp);
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(mouseMovement);
	glutMotionFunc(mouseMovement);
	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glutReshapeFunc(reshape);         // Register callback handler for window re-size event
	initGL();
	glutMainLoop();                   // Enter the infinite event-processing loop
	return 0;
}