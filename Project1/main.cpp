#include <windows.h>  // for MS Windows
#include <GL\freeglut.h>  // GLUT, include glu.h and gl.h
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <fstream>		    // File library
#include <iostream>
#include <vector>
using namespace std;

class ObjetoOpenGL
{
public:
	int tipo;
	int x, y, z;
	float r, g, b;
	float p1, p2, p3;

	ObjetoOpenGL(int Tipo, int X, int Y, int Z, float R, float G, float B) {
		tipo = Tipo;
		x = X;
		y = Y;
		z = Z;
		r = R;
		g = G;
		b = B;
	}

	void distribuirParametros(float parametros[3]) {
		p1 = parametros[0];
		p2 = parametros[1];
		p3 = parametros[2];
	}
};

class ObjetoCompostoOpenGL
{
public:
	char nome[50];
	std::vector<ObjetoOpenGL> partes;

	ObjetoCompostoOpenGL(char Nome[50]) {
		memcpy(nome, Nome, sizeof(char)*50);

	}
};

char title[64] = "OpenGL-PUCPR - Formas geométricas";
int RESOLUTION_INITIAL_WIDTH = 1600;
int RESOLUTION_INITIAL_HEIGHT = 900;

GLfloat nRange = 20.0f, angleV = 70.0f, fAspect;
GLfloat angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f; // Arrow keys user-defined rotation
GLfloat rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f; // The final global rotation (with added animation)

bool animate, animateX, animateY, animateZ, polygonMode, front, back, cface, projMode, globalIllumination = false, depthTest = true;
bool wKey, aKey, sKey, dKey, spaceKey, eKey, upKey, leftKey, rightKey, downKey, pgDnKey, pgUpKey, rClick, lClick, escKey;
bool shadeModel = true;
int forma = 1;
int frame = 0;
int
mouseX, // live mouse position X axis updated every frame
mouseY, // live mouse position Y axis
rClickX, // mouse right click X position updates on right click and/or right click dragging
rClickY, // mouse right click Y position
lClickX, // mouse left click X position updates on left click (no dragging)
lClickY, // mouse left click Y position
mouseMovedX, mouseMovedY, lastX, lastY; // camera movement mouse variables
float rAngle, time, time1, framerate, frametime, lasttime, calculatedFramerate, calculatedFrametime;
float versorVisionX, versorVisionY, versorVisionZ, visionMag,
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
speed = 0.2f,
cameraSensitivity = 0.1f,
matrizModelview[16];

std::vector<ObjetoCompostoOpenGL> Objetos;

void DisplayFileRead(const char* fileName) // na versao 2015 (char * fileName)
{	
	/*	Formato arquivo:
	numObjects
	nome
	numPartes
	tipo x y z r g b numParametros
	parametro[0]
	parametro[...]
	parametro[numParametros] // Ultimo item do objeto individual
	nome
	...
	*/
	int numObjects, numPartes;
	char nome[50];
	int tipo, x, y, z, numParametros;
	float r, g, b, parametros[3];  // cores dos objetos

	fstream inStream;
	inStream.open(fileName, ios::in); // abre o arquivo
	if (inStream.fail()) return;      //falha na abertura do arquivo
	cout << "Abertura do arquivo com sucesso ..." << endl;
	inStream >> numObjects;			  // le primeira linha do arquivo, numero de objetos 
	cout << numObjects << " Objetos na cena ..." << endl;

	for (int i = 1; i <= numObjects; i++) { // leitura das demais linhas, ID dos objetos, posicao e cor
		inStream >> nome; // Recebe nome
		ObjetoCompostoOpenGL novoObjeto(nome); // Cria objeto composto com nome recebido
		inStream >> numPartes; // Recebe numero de partes que compoem o objeto
		for (int j = 1; j <= numPartes; j++) // Para cada parte
		{
			inStream >> tipo >> x >> y >> z >> r >> g >> b >> numParametros; // Lê um objeto open gl (uma parte)
			ObjetoOpenGL parte(tipo, x, y, z, r, g, b); // Cria a parte
			for (int k = 0; k < numParametros; k++) // Para cada parametro adicional, le uma linha nova com cada parametro
			{
				inStream >> parametros[k];
			}
			parte.distribuirParametros(parametros); // distribui os parametros lidos para as variaveis dentro do objeto 'parte'
			novoObjeto.partes.push_back(parte); // mexendo na variável pública para fins de teste
		}
		Objetos.push_back(novoObjeto);
	}

	inStream.close();				 // fecha o arquivo
}

//void update(/*int value*/) {
//	rAngle += 0.2f * animate;
//	if (rAngle > 360) rAngle -= 360;
//	glutPostRedisplay(); // Inform GLUT that the display has changed
//	/*glutTimerFunc(8, update, 0);*///Call update after each 25 millisecond
//}

void initGL() {
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClearDepth(1.0f);                   // Set background depth to farthest
	//glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
	glShadeModel(GL_SMOOTH);     // Enable smooth shading
	//glEnable(GL_NORMALIZE);
}

float toRadians(float angle) {
	return angle * M_PI / 180;
}

float toDegrees(float radianAngle)
{
	return radianAngle / (M_PI / 180);
}

void renderString(float x, float y, void* font, const char* string) {

	glRasterPos2f(x, y);
	glutBitmapString(font, (unsigned char*)string);
}

void renderString3D(float x, float y, float z, void* font, const char* string) {

	glRasterPos3f(x, y, z);
	glutBitmapString(font, (unsigned char*)string);
}

void renderStrokeString(float x, float y, const char* string, float scale, bool centered = false) {

	void* font = GLUT_STROKE_MONO_ROMAN;
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

void processSpecialKeys(int key, int x, int y) {
	printf("%d   -   %d, %d\n", key, x, y);
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
	}
}

void processSpecialKeysUp(int key, int x, int y) {
	switch (key) {
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

void processNormalKeys(unsigned char key, int x, int y) {
	printf("%c   -   %d, %d\n", key, x, y);
	switch (key) {
	case 27:
		escKey = escKey ? false : true;
	case 114:  // r
		rAngle = 0;
		angleX = 0;
		angleY = 0;
		angleZ = 0;
		rotX = 0;
		rotY = 0;
		rotZ = 0;
		break;
	case 110: // n
		animate = animate ? false : true;
		break;
	case 120: // x
		animateX = animateX ? false : true;
		break;
	case 121: // y
		animateY = animateY ? false : true;
		break;
	case 122: // z
		animateZ = animateZ ? false : true;
		break;
	case 119: // w
		wKey = true;
		break;
	case 97: // a
		aKey = true;
		break;
	case 115: // s
		sKey = true;
		break;
	case 100: // d
		dKey = true;
		break;
	case 32: // space
		spaceKey = true;
		break;
	case 101: // e
		eKey = true;
		break;
	case 102: // f front
		front = front ? false : true;
		break;
	case 98: // b back
		back = back ? false : true;
		break;
	case 99: // b back
		cface = cface ? false : true;
		cface ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		break;
	case 116:
		depthTest = depthTest ? false : true;
		depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		break;
	case 112: // p projection mode
		projMode = projMode ? false : true;
		break;
	case 105:
		globalIllumination = globalIllumination ? false : true;
		break;
	case 108:
		shadeModel = shadeModel ? false : true;
		shadeModel ? glShadeModel(GL_SMOOTH) : glShadeModel(GL_FLAT);
		break;
	case 49: // 1 
		forma = 1;
		break;
	case 50: // 2
		forma = 2;
		break;
	case 51: // 3
		forma = 3;
		break;
	case 52: // 4
		forma = 4;
		break;
	case 53: // 5
		forma = 5;
		break;
	case 54: // 5
		forma = 6;
		break;
	case 55: // 6
		forma = 7;
		break;
	}
}

void processNormalKeysUp(unsigned char key, int x, int y) {
	switch (key) {
	case 119: // w
		wKey = false;
		break;
	case 97: // a
		aKey = false;
		break;
	case 115: // s
		sKey = false;
		break;
	case 100: // d
		dKey = false;
		break;
	case 32: // spacebar
		spaceKey = false;
		break;
	case 101: // e
		eKey = false;
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	printf("%d Button %s At %d %d\n", button, (state == GLUT_DOWN) ? "Down" : "Up", x, y);

	if (button == 0) // left click
	{
		// Coordinates are saved and clicked state is used by other functions
		lClick = true;
		lClickX = x;
		lClickY = y;
	}

	if (button == 2) // right click
	{
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
		// scroll wheel changes nRange when in orthogonal projection and changes camera view angle when in perspective mode
		(button == 3) ? (projMode ? nRange -= 1.0 : angleV -= 2.0) : (projMode ? nRange += 0.5 : angleV += 1);
	}
}

void mouseMovement(int x, int y) {
	mouseX = x;
	mouseY = y;

	if (rClick)
	{
		mouseMovedX = x - lastX; // mouse position changes
		mouseMovedY = y - lastY;
		lastX = x;
		lastY = y;

		if (!escKey) // blocks camera movement when ESC menu is open
		{
			cameraPitch -= mouseMovedY * cameraSensitivity; // pitch and yaw change according to differences in mouse coordinates
			cameraYaw += mouseMovedX * cameraSensitivity;
		}

		if (cameraPitch > 80) cameraPitch = 80; // limiting pitch angle between -80 and 80
		if (cameraPitch < -80) cameraPitch = -80;

		cameraYaw = (cameraYaw > 360) ? (cameraYaw - 360) : (cameraYaw < 0) ? (cameraYaw + 360) : cameraYaw; // limiting yaw to (0, 360) interval

		lookingAtX = xPos + cos(toRadians(cameraYaw)) * cos(toRadians(cameraPitch)); // camera moves in a cylindrical coordiate system
		lookingAtY = yPos + sin(toRadians(cameraPitch));
		lookingAtZ = zPos + sin(toRadians(cameraYaw)) * cos(toRadians(cameraPitch));
	}
}

void normalize2d(float x, float y, float* returnX, float* returnY) {
	// funcao antiga mas deixarei para evitar quebrar o que ja esta funcionando
	float magnitude = sqrt(pow(x, 2) + pow(y, 2));

	*returnX = x / magnitude;
	*returnY = y / magnitude;
}

void normalizarVetor(float v[], size_t s, float* vOut) {
	float mag = 0;

	float* nV = new float[s];

	for (size_t i = 0; i < s; i++)
		mag += pow(v[i], 2);

	mag = sqrt(mag);

	if (mag == 0) mag = 1;

	for (size_t i = 0; i < s; i++)
		nV[i] = v[i] / mag;

	memcpy(vOut, nV, sizeof(1.0f) * s);

	delete[] nV;
}

//void Normaliza(float vector[3]) // normalização do vetor
//{
//	float length;
//	//Cálculo do comprimento do vetor
//	length = (float)sqrt(pow(vector[0], 2.0) + pow(vector[1], 2.0) + pow(vector[2], 2.0));
//	// Evita divisão por zero
//	if (length == 0.0f) length = 1.0f;
//	// Divide cada elemento pelo comprimento do vetor
//	vector[0] /= length;
//	vector[1] /= length;
//	vector[2] /= length;
//}

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

	//glBegin(GL_LINES);
	//glColor3f(1, 1, 0);
	//glVertex3f(0, 0, 0);
	//glVertex3f(v1[0], v1[1], v1[2]);
	//glEnd();

	//glBegin(GL_LINES);
	//glColor3f(0, 1, 0);
	//glVertex3f(0, 0, 0);
	//glVertex3f(v2[0], v2[1], v2[2]);
	//glEnd();

	//glBegin(GL_LINES);
	//glColor3f(0, 1, 0);
	//glVertex3f(0, 0, 0);
	//glVertex3f(v3[0], v3[1], v3[2]);
	//glEnd();

	//glBegin(GL_LINES);
	//glColor3f(0, 1, 0);
	//glVertex3f(0, 0, 0);
	//glVertex3f(out[0], out[1], out[2]);
	//glEnd();
}

void movement() {
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
		angleY--;
	}

	if (rightKey)
	{
		angleY++;
	}

	if (upKey)
	{
		angleX--;
	}

	if (downKey)
	{
		angleX++;
	}

	if (pgUpKey)
	{
		angleZ++;
	}

	if (pgDnKey)
	{
		angleZ--;
	}
}

float fps() {
	time = glutGet(GLUT_ELAPSED_TIME);
	if (time - time1 > 1000) {
		framerate = frame * 1000.0 / (time - time1);
		time1 = time;
		frame = 0;
	}
	return framerate;
}

float ftime() {
	time = glutGet(GLUT_ELAPSED_TIME);
	frametime = time - lasttime;
	lasttime = time;
	return frametime;
}

void definirTitle() {
	snprintf(title, sizeof title, " OpenGL-PUCPR - Formas geométricas - %dx%d  %.1fFPS %.0fms ", glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), calculatedFramerate, calculatedFrametime);
	glutSetWindowTitle(title);
}

void loadWorldOrthoProj() {
	float w = glutGet(GLUT_WINDOW_WIDTH);
	float h = glutGet(GLUT_WINDOW_HEIGHT);

	if (h == 0) h = 1;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-nRange, nRange, -nRange * h / w, nRange * h / w, -nRange, nRange);
	else
		glOrtho(-nRange * w / h, nRange * w / h, -nRange, nRange, -nRange, nRange);
}

void loadWorldPerspProj() {
	float w = glutGet(GLUT_WINDOW_WIDTH);
	float h = glutGet(GLUT_WINDOW_HEIGHT);
	fAspect = w / h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(angleV, fAspect, .1, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void plano(float y, float size, int divisoes) {
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
			glVertex3f(xmin + passoX * i, y, zmin + passoZ * (j+1));
			glVertex3f(xmin + passoX * (i+1), y, zmin + passoZ * (j+1));
			glVertex3f(xmin + passoX * (i+1), y, zmin + passoZ * j);
		}
	}
	glEnd();
}

void cubo(float a) {
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

void cone(float radius, float height, int nLados, int divisoes) {
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
	double xT, yT, zT, xB, yB, zB, lxT = 0, lyT = 0, lzT = 0, lxB = 0, lyB = 0, lzB = 0;

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

			if (firstLoop) {
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

void cilindro(float radius, float height, int nLados) {

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

void tube(float innerRadius, float height, float thickness, int nLados) {
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

void comboTubes(float innerRadius, float height, float thickness, int nLados) {

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

void draw2dBox(int bx, int by, int ux, int uy) {

	glBegin(GL_LINE_LOOP);
	glVertex2i(bx, by);
	glVertex2i(ux, by);
	glVertex2i(ux, uy);
	glVertex2i(bx, uy);
	glEnd();

}

void draw2dBoxFilled(int bx, int by, int ux, int uy) {

	glBegin(GL_POLYGON);
	glVertex2i(bx, by);
	glVertex2i(ux, by);
	glVertex2i(ux, uy);
	glVertex2i(bx, uy);
	glEnd();

}

void xyzLines() {
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); // Red - X
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(+nRange, 0.0, 0.0);
	glColor3f(0.0, 1.0, 0.0); // Green - Y
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, +nRange, 0.0);
	glColor3f(0.0, 0.0, 1.0); // Blue - Z
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, +nRange);
	glEnd();
}

void xyzLines3d(float sizeFactor = 1, float lengthFactor = 1, float thicknessFactor = 1, bool pointy = 0) {

	float finalLength = sizeFactor * lengthFactor;
	float finalThickness = sizeFactor * thicknessFactor;

	glColor3f(1.0, 0.0, 0.0); // Red - X 

	glRotatef(-90, 0.0f, 0.0f, 1.0f);
	cilindro(0.02 * finalThickness, 20 * finalLength, 10);
	glTranslatef(0, 20 * finalLength, 0);
	cone(0.1 * finalThickness, 0.4 * finalLength + pointy * 10, 10, 1);
	glTranslatef(0, -20 * finalLength, 0);
	glRotatef(90, 0.0f, 0.0f, 1.0f);

	glColor3f(0.0, 1.0, 0.0); // Green - Y

	cilindro(0.02 * finalThickness, 20 * finalLength, 10);
	glTranslatef(0, 20 * finalLength, 0);
	cone(0.1 * finalThickness, 0.4 * finalLength + pointy * 10, 10, 1);
	glTranslatef(0, -20 * finalLength, 0);

	glColor3f(0.0, 0.0, 1.0); // Blue - Z 

	glRotatef(90, 1.0f, 0.0f, 0.0f);
	cilindro(0.02 * finalThickness, 20 * finalLength, 10);
	glTranslatef(0, 20 * finalLength, 0);
	cone(0.1 * finalThickness, 0.4 * finalLength + pointy * 10, 10, 1);
	glTranslatef(0, -20 * finalLength, 0);
	glRotatef(-90, 1.0f, 0.0f, 0.0f);

}

void renderCoords() {
	glColor3f(1.0, 0.0, 0.0);
	renderString3D(nRange, 1.0f, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, "X");
	glColor3f(0.0, 1.0, 0.0);
	renderString3D(0.0f, nRange + 1, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, "Y");
	glColor3f(0.0, 0.0, 1.0);
	renderString3D(0.0f, 1.0f, nRange, GLUT_BITMAP_TIMES_ROMAN_24, "Z");
}

void drawButton(int bx, int by, int buttonWidth, int buttonHeight, const char* text, void f()) {
	float buttonMidX = buttonWidth / 2;
	float buttonMidY = buttonHeight / 2;

	float leftX = bx - buttonMidX;
	float bottomY = by - buttonMidY;
	float rightX = bx + buttonMidX;
	float topY = by + buttonMidY;

	char buffer[9];
	snprintf(buffer, sizeof(buffer), text);

	if (leftX < mouseX && mouseX < rightX && bottomY < mouseY && mouseY < topY) // mouse pointer over button
	{
		lClick ? glColor3f(0.0, 1.0, 0.0) : glColor3f(0.3, 0.3, 0.3);
		draw2dBoxFilled(leftX, bottomY, rightX, topY);
		if (lClick) f();

	}
	else
	{
		glColor3f(0.1, 0.1, 0.1);
		draw2dBoxFilled(leftX, bottomY, rightX, topY);
	}
	glColor3f(1, 1, 1);
	renderStrokeString(bx + 9, bottomY + 9, buffer, 0.22, true);
}

void resumeButton() {
	escKey = false;
}

void escapeMenu(int screenX, int screenY) {

	int buttonWidth = 200;
	int buttonHeight = 40;

	/*glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINES);
	glVertex2i(0, screenY/2);
	glVertex2i(screenX, screenY/2);
	glVertex2i(screenX/2, 0);
	glVertex2i(screenX / 2, screenY);
	glEnd();*/

	glColor3f(1.0, 1.0, 0.0);
	drawButton(screenX / 2, screenY / 2, buttonWidth, buttonHeight, "Continue", resumeButton);

	//todo
	//	dynamic button generation
	//	option button
	//	quit button
}

void renderInterface() {

	float w = glutGet(GLUT_WINDOW_WIDTH);
	float h = glutGet(GLUT_WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION); // setting up an orthogonal projection proportional to screen resolution
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1000, 1000);
	glMatrixMode(GL_MODELVIEW); // back to model view
	glLoadIdentity();

	glColor3f(1.0, 0.6, 0.0);
	char mouseBuffer[50]; // mouse-cursor-following-text buffer

	snprintf(mouseBuffer, sizeof mouseBuffer, 
		"(%d, %d)\nYaw = %.1f\nPitch = %.1f",
		mouseX, mouseY, cameraYaw, cameraPitch);

	renderString(mouseX - 5 + 7, h - mouseY - 4 - 14, GLUT_BITMAP_8_BY_13, mouseBuffer); // mouse coords, pitch, yaw shown below cursor

	if (escKey) // if ESC key is pressed, only escape menu is loaded and all movement is blocked
	{
		escapeMenu(w, h);
		return;
	}

	char buffer[1024];
	snprintf(buffer, sizeof buffer,
		"ProjectionMode = %s    AspectRatio= %f    Resolution= %.0fx%.0f    %.1fFPS    %3.0fms   global_speed_multiplier= %.3f\n"
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
		"%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
		"%s\n"
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
		forma == 1 ? "(1) [Cubo]" : "(1) Cubo",
		forma == 2 ? "(2) [Cone]" : "(2) Cone",
		forma == 3 ? "(3) [Cilindro]" : "(3) Cilindro",
		forma == 4 ? "(4) [Tubo]" : "(4) Tubo",
		forma == 5 ? "(5) [3 Tubos]" : "(5) 3 Tubos",
		forma == 6 ? "(6) [Teapot]" : "(6) Teapot",
		xPos, yPos, zPos,
		lookingAtX, lookingAtY, lookingAtZ,
		visionX, visionY, visionZ);

	glColor3f(1.0, 1.0, 0.0);
	renderString(5, h - 29, GLUT_BITMAP_9_BY_15, buffer);

	//matrizModelview[0], matrizModelview[1], matrizModelview[2], matrizModelview[3],
	//matrizModelview[4], matrizModelview[5], matrizModelview[6], matrizModelview[7],
	//matrizModelview[8], matrizModelview[9], matrizModelview[10], matrizModelview[11],
	//matrizModelview[12], matrizModelview[13], matrizModelview[14], matrizModelview[15]

	snprintf(buffer, sizeof buffer,
		"GL_MODELVIEW_MATRIX \n"
		"| %2.2f | %2.2f | %2.2f | %2.2f | \n"
		"| %2.2f | %2.2f | %2.2f | %2.2f | \n"
		"| %2.2f | %2.2f | %2.2f | %2.2f | \n"
		"| %2.2f | %2.2f | %2.2f | %2.2f | \n",
		matrizModelview[0], matrizModelview[1], matrizModelview[2], matrizModelview[3],
		matrizModelview[4], matrizModelview[5], matrizModelview[6], matrizModelview[7],
		matrizModelview[8], matrizModelview[9], matrizModelview[10], matrizModelview[11],
		matrizModelview[12], matrizModelview[13], matrizModelview[14], matrizModelview[15]
	);

	renderString(w - 300, h - 29, GLUT_BITMAP_9_BY_15, buffer);

	glPushMatrix(); // Mini camera XYZ axis
	glTranslatef(w - 70, 70, 0); // Posicionamento
	glRotatef(-cameraPitch, 1.0f, 0.0f, 0.0f);// Camera rotation 
	glRotatef(cameraYaw + 90, 0.0f, 1.0f, 0.0f);
	xyzLines3d(.5, 5, 100, 1);
	glPopMatrix();

	//float xRotationFromModelview = toDegrees(acos(matrizModelview[0]));
	//float yRotationFromModelview = toDegrees(acos(matrizModelview[5]));
	//float zRotationFromModelview = toDegrees(acos(matrizModelview[10]));
	//printf("%f\n%f\n%f\n\n", xRotationFromModelview, yRotationFromModelview, zRotationFromModelview);

	//glTranslatef(-10, -10, 0);
}

void renderWorld() {

	gluLookAt(xPos, yPos, zPos, lookingAtX, lookingAtY, lookingAtZ, 0, 1, 0);

	glGetFloatv(GL_MODELVIEW_MATRIX, matrizModelview);

	//printf(
	//	"| %2.1f | %2.1f | %2.1f | %2.1f | \n"
	//	"| %2.1f | %2.1f | %2.1f | %2.1f | \n"
	//	"| %2.1f | %2.1f | %2.1f | %2.1f | \n"
	//	"| %2.1f | %2.1f | %2.1f | %2.1f | \n\n",
	//	matrizModelview[0], matrizModelview[1], matrizModelview[2], matrizModelview[3],
	//	matrizModelview[4], matrizModelview[5], matrizModelview[6], matrizModelview[7],
	//	matrizModelview[8], matrizModelview[9], matrizModelview[10], matrizModelview[11],
	//	matrizModelview[12], matrizModelview[13], matrizModelview[14], matrizModelview[15]
	//);

	front ? glPolygonMode(GL_FRONT, GL_LINE) : glPolygonMode(GL_FRONT, GL_FILL);
	back ? glPolygonMode(GL_BACK, GL_LINE) : glPolygonMode(GL_BACK, GL_FILL);

	movement(); // movimento WASD

	angleX = (angleX >= 360) ? (angleX -= 360) : (angleX < 0) ? (angleX += 360) : angleX; // 0 <= angleX < 360
	angleY = (angleY >= 360) ? (angleY -= 360) : (angleY < 0) ? (angleY += 360) : angleY;
	angleZ = (angleZ >= 360) ? (angleZ -= 360) : (angleZ < 0) ? (angleZ += 360) : angleZ;

	rotX = angleX += 0.5 * animate * animateX; // rotX = ângulo modificado pelas setas * speed factor(0.5) * bool animação global * bool animação em eixo específico 
	rotY = angleY += 0.5 * animate * animateY;
	rotZ = angleZ += 0.5 * animate * animateZ;

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

	// Habilita a definição da cor do material a partir da cor corrente

	if (globalIllumination)
	{
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHT0);

		glTranslatef(posicaoLuz[0], posicaoLuz[1], posicaoLuz[2]);
		glColor3f(1, 1, 1);
		cubo(0.2);
		glTranslatef(-posicaoLuz[0], -posicaoLuz[1], -posicaoLuz[2]);

	}
	else
	{
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHT0);
	}

	globalIllumination ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);

	glColor3f(0.3, 0.3, 0.3);
	plano(-20, 100, 300); // plano fixo

	glRotatef(rotX, 1.0f, 0.0f, 0.0f); // global rotation 
	glRotatef(rotY, 0.0f, 1.0f, 0.0f);
	glRotatef(rotZ, 0.0f, 0.0f, 1.0f);

	glDisable(GL_LIGHTING);
	xyzLines();
	//xyzLines3d();
	renderCoords();
	
	globalIllumination ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);

	switch (forma)
	{
	case 1:
		glColor3f(1, 0.7, 0);
		cubo(10.0);
		break;
	case 2:
		glColor3f(1, 0.7, 0);
		cone(8.0, 15.0, 360, 80);
		break;
	case 3:
		glColor3f(1, 0.7, 0);
		cilindro(5.0, 1.0, 36);
		break;
	case 4:
		glColor3f(1, 0.7, 0);
		tube(3.0, 10.0, 1.0, 36);
		break;
	case 5:
		glColor3f(1, 0.7, 0);
		comboTubes(3.0, 25.0, 1.4, 36);
		break;
	case 6:
		glColor3f(0.0f, 0.0f, 1.0f);
		glutSolidTeapot(15.0f);
		break;
	case 7:
		glColor3f(1.0f, 0.0f, 1.0f);
		tetraHedro();
		break;
		/*case 8:
				glColor3f(1.0f, 0.0f, 1.0f);
				tetraHedro2();*/
	}

}

void render() {
	frame++; // add a frame for framerate math
	calculatedFrametime = ftime();
	calculatedFramerate = fps();
	definirTitle();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

	//3D
	depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST); // preparing 3d world rendering
	projMode ? loadWorldOrthoProj() : loadWorldPerspProj(); // loading chosen projection
	if (!projMode)
	{
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	glMatrixMode(GL_MODELVIEW); // swapping back to model view matrix
	glLoadIdentity(); // load identity matrix
	renderWorld(); // 3d stuff

	// 2D
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	renderInterface();

	glutSwapBuffers();
	glutPostRedisplay();

	lClick = false;
	speed = calculatedFrametime * 0.0284f;
}

void reshape(GLsizei w, GLsizei h) {
	if (h == 0) h = 1;
	// Especifica as dimensões da Viewport
	glViewport(0, 0, w, h);
	// Inicializa o sistema de coordenadas
}

int main(int argc, char** argv) {
	DisplayFileRead("df.txt");              // se estiver aqui, le somente uma vez

	glutInit(&argc, argv);            // Initialize GLUT
	glutInitDisplayMode(GLUT_DOUBLE); // Enable double buffered mode
	glutInitWindowSize(RESOLUTION_INITIAL_WIDTH, RESOLUTION_INITIAL_HEIGHT);     // Set the window's initial width & height
	glutInitWindowPosition(100, 100);   // Position the window's initial top-left corner
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
	//glutIdleFunc(render);
	glutReshapeFunc(reshape);         // Register callback handler for window re-size event
	initGL();
	// Our own OpenGL initialization
	//glutTimerFunc(8, update, 0);
	glutMainLoop();                   // Enter the infinite event-processing loop
	return 0;
}