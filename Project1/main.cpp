#include <windows.h>  // for MS Windows
#include <GL\freeglut.h>  // GLUT, include glu.h and gl.h
#define _USE_MATH_DEFINES
#include <cmath>
#include <string>

char title[64] = "OpenGL-PUCPR - Formas geométricas" ;
int RESOLUTION_STARTING_WIDTH = 1600;
int RESOLUTION_STARTING_HEIGHT = 900;

GLfloat nRange = 20.0f, angleV = 70.0f, fAspect; 
GLfloat angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f; // Arrow keys user-defined rotation
GLfloat rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f; // The final global rotation (with added animation)

bool animate, animateX, animateY, animateZ, polygonMode, front, back, cface, projMode;
bool wKey, aKey, sKey, dKey, spaceKey, eKey, upKey, leftKey, rightKey, downKey, pgDnKey, pgUpKey, rClick;
int forma = 1;
int frame = 0;
int mouseX, mouseY, rClickX, rClickY, mouseMovedX, mouseMovedY, lastX, lastY;
float rAngle, time, time1, framerate, frametime, lasttime;
float versorVisionX, versorVisionY, versorVisionZ, visionMag, cameraPitch = 0.0f, cameraYaw;
float 
	lookingAtX = 0,
	lookingAtY = 0, 
	lookingAtZ = 0,
	visionX = 0,
	visionY = 0, 
	visionZ = 0, 
	xPos = 0, 
	yPos = 0, 
	zPos = 1, 
	speed = 0.1f;

bool firstStart = true;

//void update(/*int value*/) {
//	rAngle += 0.2f * animate;
//	if (rAngle > 360) rAngle -= 360;
//	glutPostRedisplay(); // Inform GLUT that the display has changed
//	/*glutTimerFunc(8, update, 0);*///Call update after each 25 millisecond
//}

void initGL() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClearDepth(1.0f);                   // Set background depth to farthest
	glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
	glShadeModel(GL_FLAT);     // Enable smooth shading
}

float toRadians(float angle) {
	return angle * M_PI / 180;
}

float toDegrees(float radianAngle)
{
	return radianAngle / (M_PI / 180);
}

void RenderString(float x, float y, void* font, const char* string) {

	const unsigned char* t = reinterpret_cast<const unsigned char*>(string);
	glColor3f(1.0, 1.0, 0.0);
	glRasterPos2f(x, y);
	glutBitmapString(font, t);
}

void RenderString3D(float x, float y, float z, void* font, const char* string) {

	const unsigned char* t = reinterpret_cast<const unsigned char*>(string);
	glRasterPos3f(x, y, z);
	glutBitmapString(font, t);
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
	case 112: // p projection mode
		projMode = projMode ? false : true;
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
	printf("Button %s At %d %d\n", (state == GLUT_DOWN) ? "Down" : "Up", x, y);
	if ((button == 3) || (button == 4))
	{
		if (state == GLUT_UP) return;
		(button == 3) ? (projMode ? nRange -= 0.5 : angleV -=1) : (projMode ? nRange += 0.5 : angleV += 1);
	}
	if (button == 2)
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
}

void mouseMovement(int x, int y) {
	mouseX = x;
	mouseY = y;
	
	if (rClick)
	{
		float sensitivity = 0.1;
		mouseMovedX = x - lastX;
		mouseMovedY = y - lastY;
		
		float viewAngleY;
		//versorVisionX
		viewAngleY = (toDegrees( acos(versorVisionY))-90);
		//versorVisionZ 
		// (visionY = (lookingAtY - yPos)) / visionMag;
		printf("\n", viewAngleY);

		cameraPitch += mouseMovedY * sensitivity ;
		cameraYaw -= mouseMovedX * sensitivity;

		if (cameraPitch > 80) cameraPitch = 80;
		if (cameraPitch < -80) cameraPitch = -80;

		printf("cameraPitch = %f sin(toRadians(cameraPitch)) = %f viewAngleY = %f\n", cameraPitch, sin(toRadians(cameraPitch)), viewAngleY);
		printf("lookingAtY %f = sin(toRadians(cameraPitch)) %f * visionMag %f\n", lookingAtY, sin(toRadians(cameraPitch)), visionMag);
		printf("versorVisionY %f = (visionY %f = (lookingAtY %f - yPos %f)) / visionMag %f\n", versorVisionY, visionY, lookingAtY, yPos, visionMag);
		
		lookingAtX = xPos + cos(toRadians(cameraYaw)) * cos(toRadians(cameraPitch));
		lookingAtY = yPos + sin(toRadians(cameraPitch));
		lookingAtZ = zPos + sin(toRadians(cameraYaw)) * cos(toRadians(cameraPitch));
		
		//lookingAtY -= mouseMovedY;

		lastX = x;
		lastY = y;
	}
}

void movement() {
	if (wKey)
	{
		xPos += versorVisionX * speed;
		yPos += versorVisionY * speed;
		zPos += versorVisionZ * speed;
		lookingAtX += versorVisionX * speed;
		lookingAtY += versorVisionY * speed;
		lookingAtZ += versorVisionZ * speed;
	}

	if (aKey)
	{
		xPos += (versorVisionX * cos(90 * M_PI / 180) + versorVisionZ * sin(90 * M_PI / 180)) * speed;
		//yPos = versorVisionY * speed;
		zPos += (-versorVisionX * sin(90 * M_PI / 180) + versorVisionZ * cos(90 * M_PI / 180)) * speed;

		lookingAtX += (versorVisionX * cos(90 * M_PI / 180) + versorVisionZ * sin(90 * M_PI / 180)) * speed;
		//lookingAtY += versorVisionY * speed;
		lookingAtZ += (-versorVisionX * sin(90 * M_PI / 180) + versorVisionZ * cos(90 * M_PI / 180)) * speed;
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

	if (dKey)
	{
		xPos -= (versorVisionX * cos(90 * M_PI / 180) + versorVisionZ * sin(90 * M_PI / 180)) * speed;
		//yPos -= versorVisionY * speed;
		zPos -= (-versorVisionX * sin(90 * M_PI / 180) + versorVisionZ * cos(90 * M_PI / 180)) * speed;
		lookingAtX -= (versorVisionX * cos(90 * M_PI / 180) + versorVisionZ * sin(90 * M_PI / 180)) * speed;
		//lookingAtY -= versorVisionY * speed;
		lookingAtZ -= (-versorVisionX * sin(90 * M_PI / 180) + versorVisionZ * cos(90 * M_PI / 180)) * speed;
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
	snprintf(title, sizeof title, " OpenGL-PUCPR - Formas geométricas - %dx%d  %.1fFPS %.0fms ", glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), fps(), ftime());
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
	gluPerspective(angleV, fAspect, .1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(1, 100, 100, 0, 0, 0, 0, 1, 0);
}

void cubo(float a) {
	glColor3f(.5, 0.2, 0.1);
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

	glColor3f(0.4, 0.1, 0.1);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(-a / 2, a / 2, -a / 2);
	glVertex3f(-a / 2, a / 2, a / 2);
	glVertex3f(a / 2, a / 2, -a / 2);
	glVertex3f(a / 2, a / 2, a / 2);
	glEnd();

	glColor3f(0.4, 0.2, 0.1);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex3f(a / 2, -a / 2, -a / 2);
	glVertex3f(a / 2, -a / 2, a / 2);
	glVertex3f(-a / 2, -a / 2, -a / 2);
	glVertex3f(-a / 2, -a / 2, a / 2);
	glEnd();
}

void cone(float radius, float height, int nLados) {

	double x, z;
	glColor3f(1, 0.7, 0);
	glBegin(GL_TRIANGLE_FAN); //BASE
	glVertex3f(0.0, 0.0, 0.0);// centro
	for (double angle = (2.0 * M_PI); angle > 0.0; angle -= (2.0 * M_PI / nLados))
	{
		x = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(x, 0.0, z);
	}
	glEnd();

	glColor3f(0.7, 0.9, 0.3);
	glBegin(GL_TRIANGLE_FAN); // LATERAL
	glVertex3f(0.0, height, 0.0); // centro
	for (double angle = 0.0; angle < (2.0 * M_PI); angle += (2.0 * M_PI / nLados))
	{
		x = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(x, 0.0, z);
	}
	glEnd();

}

void cilindro(float radius, float height, int nLados) {

	double x, z;
	glColor3f(0.4, 0.9, 0);
	glBegin(GL_TRIANGLE_FAN); // BASE
	glVertex3f(0.0, 0.0, 0.0); // centro
	for (double angle = (2.0 * M_PI); angle > 0.0; angle -= (2.0 * M_PI / nLados))
	{
		x = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(x, 0.0, z);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN); //TOPO
	glVertex3f(0.0, height, 0.0); // centro
	for (double angle = 0.0; angle < (2.0 * M_PI); angle += (2.0 * M_PI / nLados))
	{
		x = radius * sin(angle);
		z = radius * cos(angle);
		glVertex3f(x, height, z);
	}
	glEnd();

	glColor3f(0.2, 0.2, 0.7);
	glBegin(GL_TRIANGLE_STRIP); // LATERAL

	for (double angle = (2.0 * M_PI); angle > 0.0; angle -= (2.0 * M_PI / nLados))
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
	for (double angle = 0.0; angle < (2.0 * M_PI); angle += (2.0 * M_PI / nLados))
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
	for (double angle = (2.0 * M_PI); angle > 0.0; angle -= (2.0 * M_PI / nLados))
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
	for (double angle = (2.0 * M_PI); angle > 0.0; angle -= (2.0 * M_PI / nLados))
	{
		x = innerRadius * sin(angle);
		z = innerRadius * cos(angle);
		glVertex3f(x, height, z);
		glVertex3f(x, 0.0, z);

	}
	glEnd();

	glBegin(GL_TRIANGLE_STRIP); // LATERAL EXTERIOR
	glColor3f(0.7, 0.4, 0.1);
	for (double angle = (2.0 * M_PI); angle > 0.0; angle -= (2.0 * M_PI / nLados))
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

void xyzLines() {
	// Drawing XYZ Axis
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); // Red - X (to rigth)
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(+nRange, 0.0, 0.0);
	glColor3f(0.0, 1.0, 0.0); // Green - Y (to up)
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, +nRange, 0.0);
	glColor3f(0.0, 0.0, 1.0); // Blue - Z (leaving the screen)
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, +nRange);
	glEnd();
}

void renderCoords() {
	glColor3f(1.0, 0.0, 0.0);
	RenderString3D(nRange + 1, 0.0f, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, "X");
	glColor3f(0.0, 1.0, 0.0);
	RenderString3D(0.0f, nRange + 1, 0.0f, GLUT_BITMAP_TIMES_ROMAN_24, "Y");
	glColor3f(0.0, 0.0, 1.0);
	RenderString3D(0.0f, 0.0f, nRange + 1, GLUT_BITMAP_TIMES_ROMAN_24, "Z");
}

void renderInterface() {

	frame++;
	float w = glutGet(GLUT_WINDOW_WIDTH);
	float h = glutGet(GLUT_WINDOW_HEIGHT);
	//printf("%f   %f   %d   %d\n", h, rAngle, animate, animateX);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);

	glMatrixMode(GL_MODELVIEW);        // To operate on model-view matrix
	glLoadIdentity();

	char mouseBuffer[16];
	snprintf(mouseBuffer, sizeof mouseBuffer, "+ (%d, %d)", mouseX, mouseY);

	RenderString(mouseX-5, h-mouseY-4, GLUT_BITMAP_HELVETICA_18, mouseBuffer);

	char buffer[1024];
	snprintf(buffer, sizeof buffer,
		"ProjectionMode = %s    AspectRatio= %f    Resolution= %.0fx%.0f    %.1fFPS    %.0fms\n"
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
		"GL_(C)ULL_FACE = %s\n\n"
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
		"visionZ= %f\n"
		"versorVisionX= %f\n"
		"versorVisionY= %f\n"
		"versorVisionZ= %f\n"
		"lookingAtMag= %f\n",
		projMode ? "glOrtho" : "gluPerspective",
		(w / h), w, h, fps(), ftime(),
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
		forma == 1 ? "(1) [Cubo]" : "(1) Cubo",
		forma == 2 ? "(2) [Cone]" : "(2) Cone",
		forma == 3 ? "(3) [Cilindro]" : "(3) Cilindro",
		forma == 4 ? "(4) [Tubo]" : "(4) Tubo",
		forma == 5 ? "(5) [3 Tubos]" : "(5) 3 Tubos",
		xPos, yPos, zPos,
		lookingAtX, lookingAtY, lookingAtZ,
		visionX, visionY, visionZ,
		versorVisionX, versorVisionY, versorVisionZ,

		visionMag);

	RenderString(5, h-29, GLUT_BITMAP_HELVETICA_18, buffer);
	
	definirTitle();
}

void renderWorld() {
	//update();

	//if (firstStart)
	//{
	//	firstStart = false;
	//	
	//}

	//lookingAtX = xPos + versorVisionX;
	//printf("lookingAtX = %f  xPos = %f  versorVisionX = %f\n", lookingAtX, xPos, versorVisionX);
	//lookingAtY = yPos + versorVisionY;
	//lookingAtZ = zPos + versorVisionZ;

	projMode ? loadWorldOrthoProj() : loadWorldPerspProj();
	
	glMatrixMode(GL_MODELVIEW);        // To operate on model-view matrix
	glLoadIdentity();                  // Reset the model-view matrix

	if (!projMode) gluLookAt(xPos, yPos, zPos, lookingAtX, lookingAtY, lookingAtZ, 0, 1, 0);

	front ? glPolygonMode(GL_FRONT, GL_LINE) : glPolygonMode(GL_FRONT, GL_FILL);
	back ? glPolygonMode(GL_BACK, GL_LINE) : glPolygonMode(GL_BACK, GL_FILL);

	movement();

	float x = fmod(angleX += 0.5 * animate * animateX, 360);
	float y = fmod(angleY += 0.5 * animate * animateY, 360);
	float z = fmod(angleZ += 0.5 * animate * animateZ, 360);

	rotX = x = x < 0 ? x + 360 : x;
	rotY = y = y < 0 ? y + 360 : y;
	rotZ = z = z < 0 ? z + 360 : z;

	glRotatef(x, 1.0f, 0.0f, 0.0f);
	glRotatef(y, 0.0f, 1.0f, 0.0f);
	glRotatef(z, 0.0f, 0.0f, 1.0f);

	visionMag = sqrt(pow(visionX, 2) + pow(visionY, 2) + pow(visionZ, 2));
	versorVisionX = (visionX = (lookingAtX - xPos)) / visionMag;
	versorVisionY = (visionY = (lookingAtY - yPos)) / visionMag;
	versorVisionZ = (visionZ = (lookingAtZ - zPos)) / visionMag;

	xyzLines();
	renderCoords();

	//glRotatef(-rAngle, 1.0f, 1.0f, 0.0f);
	//comboTubes(3.0, 25.0, 1.4, 36);
	switch (forma)
	{
	case 1:
		cubo(10.0);
		break;
	case 2:
		cone(8.0, 15.0, 36);
		break;
	case 3:
		cilindro(5.0, 10.0, 36);
		break;
	case 4:
		tube(3.0, 10.0, 1.0, 36);
		break;
	case 5:
		comboTubes(3.0, 25.0, 1.4, 36);
		break;
	}

	//glTranslatef(5.0, 5.0, 5.0);
	//cubo(10.0);
	//glTranslatef(15.0, -5.0, 0.0);
	//cone(8.0, 15.0, 36);
	//glTranslatef(-30.0, 0.0, -5.0);
	//cilindro(5.0, 10.0, 36);

}

/* Handler for window-repaint event. Called back when the window first appears and
whenever the window needs to be re-painted. */
void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

	renderWorld();
	renderInterface();
	
	glutSwapBuffers();
	glutPostRedisplay();
}

void reshape(GLsizei w, GLsizei h) {
	if (h == 0) h = 1;
	// Especifica as dimensões da Viewport
	glViewport(0, 0, w, h);
	// Inicializa o sistema de coordenadas
}

/* Main function: GLUT runs as a console application starting at main() */
int main(int argc, char** argv) {
	glutInit(&argc, argv);            // Initialize GLUT
	glutInitDisplayMode(GLUT_DOUBLE); // Enable double buffered mode
	glutInitWindowSize(RESOLUTION_STARTING_WIDTH, RESOLUTION_STARTING_HEIGHT);     // Set the window's initial width & height
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
	//glutIdleFunc(render);
	glutReshapeFunc(reshape);         // Register callback handler for window re-size event
	initGL();   
	// Our own OpenGL initialization
	//glutTimerFunc(8, update, 0);
	glutMainLoop();                   // Enter the infinite event-processing loop
	return 0;
}