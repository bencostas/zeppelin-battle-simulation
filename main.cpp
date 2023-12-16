/*******************************************************************
		   Assignment 3 - CPS511 - Ben Costas 501025364
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include <utility>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int vWidth = 850;    // Viewport width in pixels
const int vHeight = 700;    // Viewport height in pixels

// Zeppelin variables
float zepBodyWidth = 8.0;
float zepAngle = 0.0;
float propellorRotation = 0.0;
float sidePropellorRotation = 0.0;
float driveShaftRotation = 0.0;

double zepPositionX = -40.0;
double zepPositionY = 5.0;
double zepPositionZ = 0.0;

double missilePositionX = -40.0;
double missilePositionY = 7.0;
double missilePositionZ = 0.0;
float missileAngle = 0.0;
int missileDistance = 0;

int playerHealth = 4;

bool missileFiring = false;

// Enemy Variables
double enemyPositionX = 30.0;
double enemyPositionY = 5.0;
double enemyPositionZ = 1.0;
double enemyShaftRotation = 0.0;
double enemySidePropellor = 0.0;
double enemyPropellor = 0.0;
float enemyAngle = 0.0;

double enemyMissilePositionX = 30.0;
double enemyMissilePositionY = 7.0;
double enemyMissilePositionZ = 0.0;

float enemyMissileAngle = 0.0;
int enemyMissileDistance = 0;
int enemyHealth = 4;

// Smoke Coordinates:

// Smoke Coloring:
GLfloat smoke_mat_ambient[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat smoke_mat_specular[] = { 0.3f,0.3f,0.2f,1.0f };
GLfloat smoke_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat smoke_mat_shininess[] = { 10.0F };

// Zeppelin Coloring
GLfloat playerBody_mat_ambient[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat playerBody_mat_specular[] = { 0.3f,0.55f,0.45f,1.0f };
GLfloat playerBody_mat_diffuse[] = { 0.1f,0.35f,0.6f,1.0f };
GLfloat playerBody_mat_shininess[] = { 30.0F };


GLfloat missile_mat_specular[] = { 10.3f,0.55f,0.45f,1.0f };
GLfloat missile_mat_diffuse[] = { 10.3f,0.55f,0.45f,1.0f };
GLfloat missile_mat_shininess[] = { 8000.0F };

// Enemy Coloring
GLfloat enemyBody_mat_ambient[] = { 1.5f,0.2f,0.0f,1.0f };
GLfloat enemyBody_mat_specular[] = { 1.0f,0.55f,1.0f,1.0f };
GLfloat enemyBody_mat_diffuse[] = { 0.1f,0.35f,0.6f,1.0f };
GLfloat enemyBody_mat_shininess[] = { 32.0F };

// Light properties
GLfloat light_position0[] = { 8.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 2.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.4F, 0.2F, 0.2F, 1.0F };


// Camera Toggle
bool toggleCamera = true;

// Mouse button
int currentButton;

// 3D Functions
typedef struct Vector3D
{
	GLfloat x, y, z;
} Vector3D;

// Default Mesh Size
int meshSize = 16;

#define M_PI 3.1415926


// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void animationHandler(int param);

void drawZep();
void drawPropellor(double x, double y, double z, double r);
void drawSidePropellor(double x, double y, double z, double d, double r);

// Change this to the Mesh Rendering
void drawBody(double x, double y, double z, GLuint filename);
void drawGround();
void drawStreet();

// Enemy Zeppelin Functions
void drawEnemy();
void enemyMovement(int v);
void drawMissile(double x, double y, double z, float m);
void fireMissile(double x, double y, double z, double a, double b, double c);
void fireEnemyMissile(double x, double y, double z, double a, double b, double c);
void detectCollision(double x, double y, double z, double a, double b, double c);
void detectPlayerCollision(double x, double y, double z, double a, double b, double c);
void drawSmoke(double x, double y, double z);
void drawSmoke2(double x, double y, double z);
void drawSmoke3(double x, double y, double z);

int main(int argc, char** argv)
{

	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 300);
	glutCreateWindow("Assignment 3");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);


	glutTimerFunc(1, enemyMovement, 0);
	// Start event loop, never returns
	glutMainLoop();

	return 0;
}

// --- RENDERING OBJ MESH ---

#define NUMBEROFSIDES 16

typedef struct Vertex
{
	GLfloat x, y, z;
	Vector3D normal; // normal vector for this vertex
	int numQuads;
	int quadIndex[4]; // index into quad array - tells us which quads share this vertex
} Vertex;

Vector3D* positions;  // vertex positions - used for VBO draw
Vector3D* normals;    // normal vectors for each vertex - used for VBO draw
GLuint* indices;

unsigned int numTris = 0;
unsigned int numVertices = 0;
unsigned int numIndices = 0;

void readOBJ()
{
	char buf[1024];
	char key[1024];
	int n;
	FILE* fin;

	int fc = 0; // face count
	int vc = 0; // vertex count
	int nc = 0; // normal count

	if ((fin = fopen("mesh.obj", "r")))
	{
		while (fgets(buf, 1024, fin))
			if (sscanf(buf, "%s%n", key, &n) >= 1)
			{
				if (!strcmp(key, "f"))
				{
					fc += 3;
				}
				else if (!strcmp(key, "v"))
				{
					vc++;
				}
				else if (!strcmp(key, "vn"))
				{
					nc++;
				}
			}
		fclose(fin);
	}

	positions = (Vector3D*)malloc(vc * sizeof(Vector3D));
	normals = (Vector3D*)malloc(nc * sizeof(Vector3D));
	indices = (GLuint*)malloc(fc * sizeof(GLuint));


	fc = 0; // face count
	vc = 0; // vertex count
	nc = 0; // normal count
	if ((fin = fopen("mesh.obj", "r")))
	{
		/* Process each line of the OBJ file, invoking the handler for each. */

		while (fgets(buf, 1024, fin))
			if (sscanf(buf, "%s%n", key, &n) >= 1)
			{
				if (!strcmp(key, "f"))
				{
					sscanf(buf + 1, "%d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &indices[fc], &indices[fc + 1], &indices[fc + 2]);
					fc += 3;
				}
				else if (!strcmp(key, "v"))
				{
					sscanf(buf + 1, "%f %f %f", &positions[vc].x, &positions[vc].y, &positions[vc].z);
					vc++;
				}
				else if (!strcmp(key, "vn"))
				{
					sscanf(buf + 2, "%f %f %f", &normals[nc].x, &normals[nc].y, &normals[nc].z);
					nc++;
				}
			}
		fclose(fin);

		numTris = fc / 3;
		numIndices = fc;
		numVertices = vc;


	}
}

typedef unsigned char boolean;
boolean varrayAllocated = false;

// Use immediate mode rendering to draw the mesh as triangles rather than as quads
void drawTris()
{
	glPushMatrix();
	for (int i = 0; i < (4-1) * NUMBEROFSIDES * 2 * 3; i += 3)
	{
		
		glBegin(GL_TRIANGLES);
		Vector3D* vertexp = &positions[indices[i]];
		Vector3D* vertexn = &normals[indices[i]];
		glNormal3f(vertexn->x, vertexn->y, vertexn->z);
		glVertex3f(vertexp->x, vertexp->y, vertexp->z);

		vertexp = &positions[indices[i + 1]];
		vertexn = &normals[indices[i + 1]];
		glNormal3f(vertexn->x, vertexn->y, vertexn->z);
		glVertex3f(vertexp->x, vertexp->y, vertexp->z);

		vertexp = &positions[indices[i + 2]];
		vertexn = &normals[indices[i + 2]];
		glNormal3f(vertexn->x, vertexn->y, vertexn->z);
		glVertex3f(vertexp->x, vertexp->y, vertexp->z);
		
		glEnd();

	}

	glPopMatrix();
}

GLuint groundTex;

void drawGround()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, groundTex);

	glPushMatrix();
	
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(0.0f, 0.0f);  // Bottom-left corner of the texture
	glVertex3f(-52.0f, 0.0f, -52.0f);

	glTexCoord2f(0.0f, 1.0f);  // Top-left corner of the texture
	glVertex3f(-52.0f, 0.0f, 52.0f);

	glTexCoord2f(1.0f, 1.0f);  // Top-right corner of the texture
	glVertex3f(52.0f, 0.0f, 52.0f);

	glTexCoord2f(1.0f, 0.0f);  // Bottom-right corner of the texture
	glVertex3f(52.0f, 0.0f, -52.0f);
	glEnd();
	glPopMatrix();

}

GLuint CNtower;
GLuint eiffelTower;

GLuint apartment;

void drawStreet()
{
	GLUquadricObj* cylinder = gluNewQuadric();
	GLUquadricObj* cntower = gluNewQuadric();
	GLUquadricObj* eiffeltower = gluNewQuadric();
	GLUquadricObj* disc = gluNewQuadric();

	glBindTexture(GL_TEXTURE_2D, apartment);
	gluQuadricTexture(cylinder, GL_TRUE);
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(cylinder, GLU_SMOOTH);


	glPushMatrix();
		glTranslatef(0.0, 20.0, 0.0);
		glRotatef(90.0, 1.0, 0.0, 0);
		gluCylinder(cylinder, 4.0, 4.0, 20.0, 100, 200);
		gluDisk(disc, 0, 4.0, 40, 40);
	glPopMatrix();

	glPushMatrix();
		glTranslatef(30.0, 20.0, 20.0);
		glRotatef(90.0, 1.0, 0.0, 0);
		gluCylinder(cylinder, 4.0, 4.0, 20.0, 100, 200);
		gluDisk(disc, 0, 4.0, 40, 40);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-30.0, 20.0, -20.0);
	glRotatef(90.0, 1.0, 0.0, 0);
	gluCylinder(cylinder, 6.0, 2.0, 20.0, 100, 200);
	gluDisk(disc, 0, 6.0, 40, 40);
	glPopMatrix();

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, CNtower);
		gluQuadricTexture(cntower, GL_TRUE);
		gluQuadricDrawStyle(cntower, GLU_FILL);
		glPolygonMode(GL_FRONT, GL_FILL);
		gluQuadricNormals(cntower, GLU_SMOOTH);

		glTranslatef(30.0, 20.0, -20.0);
		glRotatef(90.0, 1.0, 0.0, 0);
		gluCylinder(cntower, 2.0, 5.0, 23.0, 100, 200);
		gluDisk(disc, 0, 2.0, 40, 40);
	glPopMatrix();

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, eiffelTower);
		gluQuadricTexture(eiffeltower, GL_TRUE);
		gluQuadricDrawStyle(eiffeltower, GLU_FILL);
		glPolygonMode(GL_FRONT, GL_FILL);
		gluQuadricNormals(eiffeltower, GLU_SMOOTH);

		glTranslatef(-30.0, 20.0, 20.0);
		glRotatef(90.0, 1.0, 0.0, 0);
		gluCylinder(eiffeltower, 1.0, 5.0, 23.0, 100, 200);
		gluDisk(disc, 0, 1.0, 40, 40);
	glPopMatrix();

	
}

GLuint playerTexture;
GLuint enemyTexture;

void LoadTexture(const char* filename, GLuint &textureID) {
	int width, height, channels;
	stbi_set_flip_vertically_on_load(false);
	unsigned char* image = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if (!image) {
		fprintf(stderr, "Error loading texture image: %s\n", stbi_failure_reason());
		exit(EXIT_FAILURE);
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// Load image data into texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	// Free image data
	stbi_image_free(image);
}
// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// LOAD TEXTURE (feel free to replace the images and try with your own .jpg)

	LoadTexture("cntower.jpg", CNtower);
	LoadTexture("eiffelTower.jpg", eiffelTower);
	LoadTexture("obama.jpg", playerTexture);
	LoadTexture("steve.jpg", enemyTexture);
	LoadTexture("sidewalk.jpg", groundTex);
	LoadTexture("apartment.jpg", apartment);
}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Create Viewing Matrix V
		// FPV
	if (toggleCamera == true) {
		gluLookAt(zepPositionX+3, zepPositionY+6, zepPositionZ, zepPositionX + 3 + 0.3 * cos(zepAngle * M_PI / 180), zepPositionY + 6, zepPositionZ - 0.3 * sin(zepAngle * M_PI / 180), 0.0, 1.0, 0.0);
	}
	else {
		// Birds Eye View
		gluLookAt(1.0, 75.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	}

	// Draw Zeppelin + Missile
	if (playerHealth < 4) {
		drawSmoke(zepPositionX, zepPositionY + 10, zepPositionZ);
	}
	if (playerHealth < 3) {
		drawSmoke2(zepPositionX, zepPositionY + 10, zepPositionZ);
	}
	if (playerHealth < 2) {
		drawSmoke3(zepPositionX, zepPositionY + 10, zepPositionZ);
	}
	if (playerHealth == 0 && zepPositionY - 1.5 > -30) {
		zepPositionY -= 1.5;
		missilePositionY -= 1.5;
	}

	drawMissile(missilePositionX, missilePositionY, missilePositionZ, missileAngle);
	drawZep();

	// Draw Enemy + Enemy Missile
	if (enemyHealth < 4) {
		drawSmoke(enemyPositionX, enemyPositionY, enemyPositionZ);
	}
	if (enemyHealth < 3) {
		drawSmoke2(enemyPositionX, enemyPositionY, enemyPositionZ);
	}
	if (enemyHealth < 2) {
		drawSmoke3(enemyPositionX, enemyPositionY, enemyPositionZ);
	}

	drawMissile(enemyMissilePositionX, enemyMissilePositionY, enemyMissilePositionZ, enemyMissileAngle);
	drawEnemy();
	//enemyMovement();

	// Draw ground
	glPushMatrix();
	glTranslatef(0.0, -20.0, 0.0);
	drawGround();
	drawStreet();
	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawZep()
{

	glPushMatrix();
	// spin zep on base. 
	glTranslatef(zepPositionX, zepPositionY, zepPositionZ);
	glRotatef(zepAngle, 0.0, 1.0, 0.0);
	glTranslatef(-zepPositionX, -zepPositionY, -zepPositionZ);
	
	drawBody(zepPositionX, zepPositionY, zepPositionZ, playerTexture);
	drawPropellor(zepPositionX, zepPositionY, zepPositionZ, propellorRotation);
	drawSidePropellor(zepPositionX, zepPositionY, zepPositionZ, driveShaftRotation, sidePropellorRotation);
	glPopMatrix();

}

void drawEnemy()
{
	glPushMatrix();

	glTranslatef(enemyPositionX, enemyPositionY, enemyPositionZ);
	glRotatef(180.0 + enemyAngle, 0.0, 1.0, 0.0);
	glTranslatef(-enemyPositionX, -enemyPositionY, -enemyPositionZ);

	drawBody(enemyPositionX, enemyPositionY, enemyPositionZ, enemyTexture);
	drawPropellor(enemyPositionX, enemyPositionY, enemyPositionZ, enemyPropellor);
	drawSidePropellor(enemyPositionX, enemyPositionY, enemyPositionZ, enemyShaftRotation, enemySidePropellor);
	
	glPopMatrix();
}
double distance(double x, double y, double z, double a, double b, double c) {
	return sqrt(pow(x - a, 2) + pow(y - b, 2) + pow(z - c, 2));
}
double yD;
double aD;
void enemyMovement(int value) {
	// if enemyHealth == 2: enemy will fall // Optional Bonus 2
	if (enemyHealth == 0) {
		if (enemyPositionY - 1.5 > -30 && playerHealth != 0) {
			enemyMissilePositionX = enemyPositionX;
			enemyMissilePositionZ = enemyPositionZ;
			enemyPositionY -= 1.5;
			enemyMissilePositionY -= 1.5;
		}
	}

	// while Enemy Zeppelin is still alive:

	// If player is within range, rotate enemy zeppelin to face player, and match y-level
	else if (distance(enemyPositionX, enemyPositionY, enemyPositionZ, zepPositionX, zepPositionY, zepPositionZ) < 35) {

		// Should match playerY coord if in range
		if (zepPositionY != enemyPositionY) {
			yD = (zepPositionY - enemyPositionY) / 2;
			enemyPositionY += yD / 10;
			enemyMissilePositionY += yD / 10;
			enemyShaftRotation += 3.0;
			if (yD < 0) {
				if (enemySidePropellor > -90) {
					enemySidePropellor -= 3.0;
				}
			}
			else {
				if (enemySidePropellor < 0) {
					enemySidePropellor += 3.0;
				}

			}
		}

		// Determine angle to rotate to zepp by using cosine law
		// Determine Temporary Point (This is where the enemy zeppelin is currently looking):
		double tx = enemyPositionX - 0.4 * cos(enemyAngle * M_PI / 180);
		double tz = enemyPositionZ + 0.4 * sin(enemyAngle * M_PI / 180);

		// calculate distance from zepToTemp, enemyToTemp, zepToEnemy then use cosine law to determine the angle
		double zepToTemp = sqrt(pow(tx - zepPositionX, 2) + pow(tz - zepPositionZ, 2));
		double enemyToTemp = sqrt(pow(tx - enemyPositionX, 2) + pow(tz - enemyPositionZ, 2));
		double zepToEnemy = sqrt(pow(enemyPositionX - zepPositionX, 2) + pow(enemyPositionZ - zepPositionZ, 2));

		double cosZET = (pow(abs(zepToEnemy), 2) + pow(abs(enemyToTemp), 2) - pow(abs(zepToTemp), 2)) / (2 * abs(zepToEnemy) * abs(enemyToTemp));
		double angleZET = acos(cosZET);

		// Determine which way to turn the enemy zeppelin:
		
		// If the zepAngle > 360 || zepAngle < -360:
		double tempAngle = zepAngle;
		if (zepAngle > 360 || zepAngle < -360) {
			int n = (int)(zepAngle / 360);
			if (zepAngle > 360) {
				tempAngle = zepAngle - 360 * n;
			}
			else {
				tempAngle = zepAngle - 360 * n;
			}
		}

		if (zepAngle < 0 && tempAngle > -180 || zepAngle > 0 && tempAngle > 180) {
			enemyAngle += 2.3 * angleZET;
			enemyMissileAngle += 2.3 *  angleZET;
		}
		else {
			enemyAngle -= 2.3 * angleZET;
			enemyMissileAngle -= 2.3*  angleZET;
		}
		
		// Fire Missile
		if (angleZET < 0.3) {
			fireEnemyMissile(enemyMissilePositionX, enemyMissilePositionY, enemyMissilePositionZ, zepPositionX, zepPositionY, zepPositionZ);

		}
		// If not in line, retract the missile
		else {
			enemyMissilePositionX = enemyPositionX;
			enemyMissilePositionZ = enemyPositionZ;
		}
		
	}
	else {
		// If no enemy within range, keep moving
		if ((enemyPositionZ + 0.4 * sin(enemyAngle * M_PI / 180) > -30 && enemyPositionZ + 0.4 * sin(enemyAngle * M_PI / 180) < 31 && enemyPositionX - 0.4 * cos(enemyAngle * M_PI / 180) > -30 && enemyPositionX - 0.4 * cos(enemyAngle * M_PI / 180) < 31)) {
			enemyPositionZ += 0.4 * sin(enemyAngle * M_PI / 180);
			enemyPositionX -= 0.4 * cos(enemyAngle * M_PI / 180);
			enemyPropellor += 3.0;

			enemyMissilePositionX = enemyPositionX;
			enemyMissilePositionZ = enemyPositionZ;
		}

		// if at a boundary, rotate the zeppelin
		else {
			enemyAngle += 3.0;
			enemyMissileAngle += 3.0;
		}
	}

	glutTimerFunc(40, enemyMovement, 0);
	glutPostRedisplay();
}

void drawBody(double zepPositionX, double zepPositionY, double zepPositionZ, GLuint textureID)
{
	
	GLUquadricObj* quadratic;
	GLUquadricObj* cylinder = gluNewQuadric();

	// For the main body, nose, tail, cabin, and fins
	glPushMatrix();
		glTranslatef(zepPositionX, zepPositionY, zepPositionZ);
		glTranslatef(-5.0, 0.0, 0.0);
		glScalef(zepBodyWidth, zepBodyWidth, zepBodyWidth);
		quadratic = gluNewQuadric();

	glPushMatrix();
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

		// Bind Texture to the body
		glBindTexture(GL_TEXTURE_2D, textureID);
		gluQuadricTexture(cylinder, GL_TRUE);
		gluQuadricDrawStyle(cylinder, GLU_FILL);
		glPolygonMode(GL_FRONT, GL_FILL);
		gluQuadricNormals(cylinder, GLU_SMOOTH);

		glRotatef(180.0, 0.0, 0.0, 1.0);
		gluCylinder(cylinder, 0.3f, 0.3f, 1.3f, 32, 32); // Body, will be texture mapped
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, textureID);
	gluQuadricTexture(quadratic, GL_TRUE);
	gluQuadricDrawStyle(quadratic, GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(quadratic, GLU_SMOOTH);

	gluSphere(quadratic, 0.3f, 32, 12); // Nose

	glPushMatrix();
		glTranslatef(1.3, 0.0, 0.0);
		gluSphere(quadratic, 0.3f, 32, 12); // Tail
	glPushMatrix();
		glTranslatef(-1.64, 0, 0);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		gluCylinder(quadratic, 0.05f, 0.05f, 0.1f, 32, 32); // Shaft
		glPopMatrix();
		glPushMatrix();
	glTranslatef(-0.1, -0.3, 0);
		glScalef(0.25 * zepBodyWidth, 0.1 * zepBodyWidth, 0.1 * zepBodyWidth);
		glutSolidCube(0.2); // Cabin
		glPopMatrix();
	glPushMatrix();
		glTranslatef(-1.3, 0, 0);
		glScalef(0.5 * zepBodyWidth, 0.04 * zepBodyWidth, 1 * zepBodyWidth);
		glutSolidCube(0.1); // Fin
	glPopMatrix();

	glPushMatrix();
		glTranslatef(-1.1, -2.7, 0);
		readOBJ();
		drawTris(); // zeppelin.obj rendered to make the zeppelin look more "blimpy"
	glPopMatrix();

	glPushMatrix();
		glTranslatef(-1.1, 0, 0);
		glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
		glScalef(0.7 * zepBodyWidth, 0.04 * zepBodyWidth, 0.7 * zepBodyWidth);
		glutSolidCube(0.1); // Fin Angle
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();

}
void drawPropellor(double zepPositionX, double zepPositionY, double zepPositionZ, double propellorRotation) {

	// create Translation matrix

	// create rotation matrix
	GLfloat rotatePropellor1[]{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos((60.0f + propellorRotation) * 3.1415 / 180), sin((60.0f + propellorRotation) * 3.1415 / 180), 0.0f,
		0.0f, -sin((60.0f + propellorRotation) * 3.1415 / 180), cos((60.0f + propellorRotation) * 3.1415 / 180), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	GLfloat rotatePropellor2[]{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos((180.0f + propellorRotation) * 3.1415 / 180), sin((180.0f + propellorRotation) * 3.1415 / 180), 0.0f,
		0.0f, -sin((180.0f + propellorRotation) * 3.1415 / 180), cos((180.0f + propellorRotation) * 3.1415 / 180), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	GLfloat rotatePropellor3[]{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos((120.0f + propellorRotation) * 3.1415 / 180), sin((120.0f + propellorRotation) * 3.1415 / 180), 0.0f,
		0.0f, -sin((120.0f + propellorRotation) * 3.1415 / 180), cos((120.0f + propellorRotation) * 3.1415 / 180), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	GLUquadricObj* sphere;

	glPushMatrix();
	glTranslatef(zepPositionX - 8, zepPositionY, zepPositionZ);
	glScalef(zepBodyWidth, zepBodyWidth, zepBodyWidth);
	sphere = gluNewQuadric();
	gluSphere(sphere, 0.06f, 32, 32); // Bearing

	glPushMatrix();
	glMultMatrixf(rotatePropellor1);
	glScalef(0.04 * zepBodyWidth, 0.3 * zepBodyWidth, 0.07 * zepBodyWidth);
	glutSolidCube(0.1); // Propellor 1
	glPopMatrix();

	glPushMatrix();
	glMultMatrixf(rotatePropellor2);
	glScalef(0.04 * zepBodyWidth, 0.3 * zepBodyWidth, 0.07 * zepBodyWidth);
	glutSolidCube(0.1); // Propellor 2
	glPopMatrix();

	glPushMatrix();
	glMultMatrixf(rotatePropellor3);
	glScalef(0.04 * zepBodyWidth, 0.3 * zepBodyWidth, 0.07 * zepBodyWidth);
	glutSolidCube(0.1); // Propellor 3
	glPopMatrix();

	glPopMatrix();
}
void drawSidePropellor(double zepPositionX, double zepPositionY, double zepPositionZ, double driveShaftRotation, double sidePropellorRotation) {
	GLUquadricObj* cylinder;
	GLUquadricObj* sphere;

	glPushMatrix();
	glTranslatef(zepPositionX, zepPositionY, zepPositionZ);
	glScalef(zepBodyWidth, zepBodyWidth, zepBodyWidth);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.38f);
	cylinder = gluNewQuadric();
	gluCylinder(cylinder, 0.03f, 0.03f, 0.76f, 32, 32);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.35f);
	glRotatef(-40.0f + sidePropellorRotation, 0.0f, 0.0f, 1.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	cylinder = gluNewQuadric();
	gluCylinder(cylinder, 0.03f, 0.03f, 0.16f, 32, 32);

	glPushMatrix();
	sphere = gluNewQuadric();
	glTranslatef(0.0f, 0.001f, 0.16f);
	gluSphere(sphere, 0.03f, 32, 32);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.15f);
	glRotatef(60.0f + driveShaftRotation, 0.0f, 0.0f, 1.0f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.04 * zepBodyWidth, 0.15 * zepBodyWidth, 0.04 * zepBodyWidth);
	glutSolidCube(0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.15f);
	glRotatef(180.0f + driveShaftRotation, 0.0f, 0.0f, 1.0f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.04 * zepBodyWidth, 0.15 * zepBodyWidth, 0.04 * zepBodyWidth);
	glutSolidCube(0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.15f);
	glRotatef(120.0f + driveShaftRotation, 0.0f, 0.0f, 1.0f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.04 * zepBodyWidth, 0.15 * zepBodyWidth, 0.04 * zepBodyWidth);
	glutSolidCube(0.1);
	glPopMatrix();

	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -0.35f);
	glRotatef(-40.0f + sidePropellorRotation, 0.0f, 0.0f, 1.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	cylinder = gluNewQuadric();
	gluCylinder(cylinder, 0.03f, 0.03f, 0.16f, 32, 32);

	glPushMatrix();
	sphere = gluNewQuadric();
	glTranslatef(0.0f, -0.001f, 0.16f);
	gluSphere(sphere, 0.03f, 32, 32);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.15f);
	glRotatef(60.0f + driveShaftRotation, 0.0f, 0.0f, 1.0f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.04 * zepBodyWidth, 0.15 * zepBodyWidth, 0.04 * zepBodyWidth);
	glutSolidCube(0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.15f);
	glRotatef(180.0f + driveShaftRotation, 0.0f, 0.0f, 1.0f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.04 * zepBodyWidth, 0.15 * zepBodyWidth, 0.04 * zepBodyWidth);
	glutSolidCube(0.1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.15f);
	glRotatef(120.0f + driveShaftRotation, 0.0f, 0.0f, 1.0f);
	glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.04 * zepBodyWidth, 0.15 * zepBodyWidth, 0.04 * zepBodyWidth);
	glutSolidCube(0.1);
	glPopMatrix();

	glPopMatrix();
	glPopMatrix();
}

void drawMissile(double x, double y, double z, float m) {

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(m, 0.0, 1.0, 0.0);
	glTranslatef(-x, -y, -z);

	glMaterialfv(GL_FRONT, GL_AMBIENT, playerBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, missile_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, missile_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, missile_mat_shininess);

	glPushMatrix();
		glTranslatef(x, y, z);
		glScalef(6.0 * zepBodyWidth, 1.0 * zepBodyWidth, 1.0 * zepBodyWidth);
		glutSolidCube(0.1);
	glPopMatrix();
	glPopMatrix();
}

void fireMissile(double x, double y, double z, double a, double b, double c) {
	// Missile starts moving in a straight line, relative to the current zepPosition
	missilePositionZ -= 0.4 * sin(missileAngle * M_PI / 180);
	missilePositionX += 0.4 * cos(missileAngle * M_PI / 180);
	missileDistance++;
	
	detectCollision(x, y, z, a, b, c);
	
	if (missileDistance == 100) {
		missileDistance = 0;
		missilePositionZ = zepPositionZ;
		missilePositionX = zepPositionX;
	}
}

void fireEnemyMissile(double x, double y, double z, double a, double b, double c) {
	// Missile starts moving in a straight line, relative to the current zepPosition
	enemyMissilePositionZ += 0.4 * sin(enemyMissileAngle * M_PI / 180);
	enemyMissilePositionX -= 0.4 * cos(enemyMissileAngle * M_PI / 180);
	enemyMissileDistance++;

	detectPlayerCollision(x, y, z, a, b, c);

	if (enemyMissileDistance == 100) {
		enemyMissileDistance = 0;
		enemyMissilePositionZ = enemyPositionZ;
		enemyMissilePositionX = enemyPositionX;
	}
}

void drawSmoke(double smokeX, double smokeY, double smokeZ) {

	glMaterialfv(GL_FRONT, GL_AMBIENT, smoke_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, smoke_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, smoke_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, smoke_mat_shininess);

	glPushMatrix();
	glTranslatef(smokeX, smokeY, smokeZ);

	glPushMatrix(); // one smoke ball
	glTranslatef(5, 8, -4);
	glutSolidSphere(4.0, 32.0, 20.0);
	glPopMatrix();

	glPopMatrix();
}

void drawSmoke2(double smokeX, double smokeY, double smokeZ) {
	glMaterialfv(GL_FRONT, GL_AMBIENT, smoke_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, smoke_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, smoke_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, smoke_mat_shininess);

	glPushMatrix();
	glTranslatef(smokeX, smokeY, smokeZ);

	glPushMatrix(); // second smoke ball
	glTranslatef(2, 10, 2);
	glutSolidSphere(4.0, 32.0, 20.0);
	glPopMatrix();

	glPopMatrix();
}

void drawSmoke3(double smokeX, double smokeY, double smokeZ) {
	glMaterialfv(GL_FRONT, GL_AMBIENT, smoke_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, smoke_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, smoke_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, smoke_mat_shininess);

	glPushMatrix();
	glTranslatef(smokeX, smokeY, smokeZ);

	glPushMatrix(); // third smoke ball
	glTranslatef(7, 5, 0);
	glutSolidSphere(4.0, 32.0, 20.0);
	glPopMatrix();
	glPopMatrix();
}

void detectCollision(double x, double y, double z, double a, double b, double c) {
	// If (x, y, z) is within certain distance from (a, b, c), consider this a hit
	printf("Enemy Health: %d\n", enemyHealth);
	if (sqrt(pow(x-a, 2) + pow(z-c, 2)) < 10 ) {
		// Decrement health points:
		
		if (enemyHealth > 0) {
			enemyHealth--;
		}

		missilePositionX = zepPositionX;
		missilePositionZ = zepPositionZ;

	}
}

void detectPlayerCollision(double x, double y, double z, double a, double b, double c) {
	// If (x, y, z) is within certain distance from (a, b, c), consider this a hit
	printf("Enemy Health: %d\n", enemyHealth);
	if (sqrt(pow(x - a, 2) + pow(z - c, 2)) < 10) {
		// Decrement health points:

		if (playerHealth > 0) {
			playerHealth--;
		}


		enemyMissilePositionX = enemyPositionX;
		enemyMissilePositionZ = enemyPositionZ;
	}
}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 220.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (toggleCamera == true) {
		gluLookAt(zepPositionX + 3, zepPositionY + 6, zepPositionZ, zepPositionX + 3 + 0.3 * cos(zepAngle * M_PI / 180), zepPositionY+6, zepPositionZ - 0.3 * sin(zepAngle * M_PI / 180), 0.0, 1.0, 0.0);
	}
	else {
		// Birds Eye View
		gluLookAt(1.0, 75.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	}
}

bool stop = false;


// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		if (!missileFiring) {
			propellorRotation += 8.0;

			zepPositionZ -= 0.4 * sin(zepAngle * M_PI / 180);
			zepPositionX += 0.4 * cos(zepAngle * M_PI / 180);

			missilePositionZ = zepPositionZ;
			missilePositionX = zepPositionX;
		}

		// if playerHealth == 2: player will fall // Optional Bonus 2
		if (playerHealth == 0) {

			missilePositionX = zepPositionX;
			missilePositionZ = zepPositionZ;
			zepPositionY -= 1.5;
			missilePositionY -= 1.5;
		}

		break;
	
		// switch camera 
	case 's':
		toggleCamera = !toggleCamera;
		break;
	
		// fire missile
	case 'f':
		missileFiring = true;
		if (missileFiring) {
			fireMissile(missilePositionX, missilePositionY, missilePositionZ, enemyPositionX, enemyPositionY, enemyPositionZ);
		}

		break;
		// Stop Missile Firing
	case 'e':
		missileFiring = false;
		missilePositionX = zepPositionX;
		missilePositionZ = zepPositionZ;

		break;
	}
	glutPostRedisplay();   // Trigger a window redisplay
}


void animationHandler(int param)
{
	if (!stop)
	{
		zepAngle += 1.0;
		glutPostRedisplay();
		glutTimerFunc(10, animationHandler, 0);
	}
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_UP) {


		if (zepPositionY + 0.15 < 10) {
			zepPositionY += 0.15;
			missilePositionY += 0.15;
		}
		driveShaftRotation += 3.0;
		if (sidePropellorRotation < 0) {
			sidePropellorRotation += 3.0;
		}

	}
	else if (key == GLUT_KEY_DOWN) {

		if (zepPositionY - 0.15 > 3) {
			zepPositionY -= 0.15;
			missilePositionY -= 0.15;
		}
		driveShaftRotation += 3.0;
		if (sidePropellorRotation > -90) {
			sidePropellorRotation -= 3.0;
		}
	}
	else if (key == GLUT_KEY_LEFT) {

		if (!missileFiring) {
			zepAngle += 2.0;
			missileAngle += 2.0;
		}
		
	}
	else if (key == GLUT_KEY_RIGHT) {

		if (!missileFiring) {
			zepAngle -= 2.0;
			missileAngle -= 2.0;
		}
		
	}
	glutPostRedisplay();   // Trigger a window redisplay
}