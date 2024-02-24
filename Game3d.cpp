#define GL_SILENCE_DEPRECATION;
#include <GLUT/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rwops.h>
#include "SDL_stdinc.h"
#include <SDL2/SDL.h>
#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)
GLfloat fenceColor[3] = { 0.0f, 0.6f, 0.0f }; // Initial fence color
int ColorTimer = 5000; // (5 seconds)
int counter=0;
bool gameOver = false;
int gameTime = 50000;
std::time_t gameStartTime;

float treeRotationAngle = 0.0f;
bool rotateTree = false;
float GoalRotationAngle = 0.0f;
float playerAngle = 0.0f; // Angle in degrees
float ferrisWheelRotationAngle = 0.0f;
bool ferrisWheel = false;


const float HEAD_RADIUS = 0.15;
const float BODY_HEIGHT = 0.4;
const float ARM_LENGTH = 0.3;
const float ARM_WIDTH = 0.1;
const float LEG_LENGTH = 0.4;
const float LEG_WIDTH = 0.1;

float wheelRadius = 1.0f; // Radius of the Ferris wheel
float supportHeight = 4.0f; // Height of the support beams
float compartmentWidth = 0.3f; // Width of the passenger compartments
float compartmentHeight = 0.3f; // Height of the passenger compartments
float compartmentSpacing = 0.2f; // Spacing between compartments

bool goalCollected = false;
float playerX = 0.0;
float playerY = 0.50;
float playerZ = 1.90;
float playerSpeed = 0.1;


Mix_Chunk *collisionSound = NULL;
Mix_Chunk *anmationSound = NULL;
Mix_Chunk *winningSound = NULL;
Mix_Chunk *goalSound = NULL;

class Vector3f {
public:
    float x, y, z;

    Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
        x = _x;
        y = _y;
        z = _z;
    }

    Vector3f operator+(Vector3f& v) {
        return Vector3f(x + v.x, y + v.y, z + v.z);
    }

    Vector3f operator-(Vector3f& v) {
        return Vector3f(x - v.x, y - v.y, z - v.z);
    }

    Vector3f operator*(float n) {
        return Vector3f(x * n, y * n, z * n);
    }

    Vector3f operator/(float n) {
        return Vector3f(x / n, y / n, z / n);
    }

    Vector3f unit() {
        return *this / sqrt(x * x + y * y + z * z);
    }

    Vector3f cross(Vector3f v) {
        return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
};
void changeFenceColor(int value) {
    // Change the fence color
    fenceColor[0] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    fenceColor[1] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    fenceColor[2] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    // Reset the timer
    glutTimerFunc(ColorTimer, changeFenceColor, 0);
}
void timer(int value) {
    unsigned int currentTime = glutGet(GLUT_ELAPSED_TIME);
    gameTime--;
    if (gameTime % ColorTimer == 0) { // Check if gameTime is a multiple of 5
           changeFenceColor(0); // Change fence color every 5 secs
       }
    if (gameTime <= 0) {
        gameOver = true;
        gameTime = 0;
    }

    glutTimerFunc(1000, timer, 0); // Set the timer to call itself every 1000 milliseconds (1 second)
    glutPostRedisplay(); // Trigger a display callback
}
void drawPavilion() {
    // Base cylinder for the pavilion
    glPushMatrix();
    glRotatef(treeRotationAngle, 0.0f, 1.0f, 0.0f);
    GLUquadricObj* quadric = gluNewQuadric();
    glColor3f(0.6f, 0.4f, 0.3f); // Brown color
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    gluCylinder(quadric, 1.0f, 1.0f, 0.5f, 30, 30);
    glPopMatrix();

    // Conical roof
    glColor3f(0.3f, 0.0f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 0.75f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Rotate 90 degrees around the x-axis
    glutSolidCone(1.0f, 1.0f, 30, 30);
    glPopMatrix();

    // Cubes for seating around the pavilion
    glColor3f(0.9f, 0.8f, 0.7f); // Light beige color
    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        glTranslatef(cos(3.14 / 2.0 * i), 0.15f, sin(3.14 / 2.0 * i));
        glutSolidCube(0.25f);
        glPopMatrix();
    }

    // Spheres for decoration
    glColor3f(0.4f, 0.3f, 1.0f);
    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        glTranslatef(cos(3.14 / 2.0 * i) * 1.1f, 0.75f, sin(3.14 / 2.0 * i) * 1.1f);
        glutSolidSphere(0.1f, 20, 20);
        glPopMatrix();
    }

    // Torus for additional decorative elements
    glColor3f(0.7f, 0.2f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidTorus(0.05f, 0.9f, 30, 30);
    glPopMatrix();
    glPopMatrix();

}


void drawPlayer() {
    glPushMatrix();

    // Translate to the player's position
    glTranslatef(playerX, playerY, playerZ);

    // Rotate the player to face the current direction
    glRotatef(playerAngle, 0.0f, 1.0f, 0.0f);  // Rotate around the y-axis

    // Draw the head
    glColor3f(1.0, 0.0, 0.0);  // Red color for the head
    glPushMatrix();
    glTranslatef(0.04, 0.0, 0.0);
    glutSolidSphere(HEAD_RADIUS * 0.6, 20, 20);
    glPopMatrix();

    // Draw the body
    glColor3f(0.0, 1.0, 0.0);  // Green color for the body
    glPushMatrix();
    glTranslatef(0.05, -0.24, 0.0);
    glScalef(1.5, 3, 1); // Scale down the body
    glutSolidCube(0.1);
    glPopMatrix();

    // Draw the left arm
    glColor3f(0.0, 0.0, 1.0);  // Blue color for the left arm
    glPushMatrix();
    glTranslatef(-0.1, -0.2, 0.0);
    glScalef(1.0, 0.2 * 2, 0.2);
    glRotatef(45.0, 0.0, 0.0, 1.0);  // Rotate the arm outward
    glutSolidCube(ARM_LENGTH * 0.6); // Scale down the arm
    glPopMatrix();

    // Draw the right arm
    glPushMatrix();
    glTranslatef(0.19, -0.2, 0.0);
    glScalef(1.0, 0.2 * 2, 0.2);
    glRotatef(-45.0, 0.0, 0.0, 1.0);  // Rotate the arm outward
    glutSolidCube(ARM_LENGTH * 0.6); // Scale down the arm
    glPopMatrix();

    // Draw the left leg
    glColor3f(0.8, 0.4, 0.0);  // Brown color for the left leg
    glPushMatrix();
    glTranslatef(0, -0.5, 0.0);
    glScalef(0.2, 1.0 * 2, 0.2);
    glutSolidCube(LEG_LENGTH * 0.6); // Scale down the leg
    glPopMatrix();

    // Draw the right leg
    glPushMatrix();
    glTranslatef(0.1, -0.5, 0.0);
    glScalef(0.2, 1.0 * 2, 0.2);
    glutSolidCube(LEG_LENGTH * 0.6); // Scale down the leg
    glPopMatrix();
    glPopMatrix();

}


class Camera {
public:
    Vector3f eye, center, up;

    Camera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
        eye = Vector3f(eyeX, eyeY, eyeZ);
        center = Vector3f(centerX, centerY, centerZ);
        up = Vector3f(upX, upY, upZ);
    }

    void moveX(float d) {
        Vector3f right = up.cross(center - eye).unit();
        Vector3f x = right * d;
        eye = eye + x;
        center = center + x;
    }

    void moveY(float d) {
        Vector3f y = up.unit() * d;
        eye = eye + y;
        center = center + y;
    }

    void moveZ(float d) {
        Vector3f view = (center - eye).unit();
        Vector3f z = view * d;
        eye = eye + z;
        center = center + z;
    }

    void rotateX(float a) {
        Vector3f view = (center - eye).unit();
        Vector3f right = up.cross(view).unit();
        Vector3f temp = up * sin(DEG2RAD(a));
        view = view * cos(DEG2RAD(a)) + temp;
        center = eye + view;
    }

    void rotateY(float a) {
        Vector3f view = (center - eye).unit();
        Vector3f right = up.cross(view).unit();
        Vector3f temp = right * sin(DEG2RAD(a));
        view = view * cos(DEG2RAD(a)) + temp;
        right = view.cross(up);
        center = eye + view;
    }



    void look() {
        gluLookAt(
            eye.x, eye.y, eye.z,
            center.x, center.y, center.z,
            up.x, up.y, up.z
        );
    }
};
class ThirdPersonCamera : public Camera {
public:
    float distance;  // Distance between the camera and the player

    ThirdPersonCamera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f,
        float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f,
        float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f,
        float distance = 1.0f)
        : Camera(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ), distance(distance) {}

    void followPlayer(Vector3f playerPosition) {
        // Calculate the camera position based on player position and the distance
        Vector3f direction = (playerPosition - center).unit();
        eye = playerPosition + direction;
        eye = eye * distance;

        // Keep the up vector fixed
        look();
    }
};

ThirdPersonCamera thirdPersonCamera;
Camera camera;

void drawWall(double thickness, double scaleFactor) {
    glPushMatrix();
    glTranslated(0, 0, 0);  // Adjust the translation values here
    glColor3f(0.0, 1.0, 0.0);
    glScaled(1.0 * scaleFactor, thickness * scaleFactor, 1.0 * scaleFactor); // Adjust the scale factor here
    glutSolidCube(1);
    glPopMatrix();
}
void setupLights() {
    GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
    GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
    GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
    GLfloat shininess[] = { 50 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

    GLfloat lightIntensity[] = { 0.7f, 0.7f, 1, 1.0f };
    GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}
void setupCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 800 / 600, 0.001, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set the camera position and orientation
    //thirdPersonCamera.eye = Vector3f(40.0f, 40.0f, 40.0f);  // Set the desired camera position
    //thirdPersonCamera.center = Vector3f(10.0f, 10.0f, 10.0f);  // Set the desired look-at point
    //thirdPersonCamera.up = Vector3f(0.0f, 1.0f, 0.0f);  // Set the up vector

    thirdPersonCamera.look();  // Apply the camera settings
}


void drawHorizontalBar(float x, float y, float z, float length, float thickness) {
    // Draw the horizontal part of the fence
   // glColor3f(0.0f, 0.6f, 0.0f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(length, thickness, thickness);
    glutSolidCube(1.0f);
    glPopMatrix();
}
void drawHorizontal2Bar(float x, float y, float z, float length, float thickness) {
    glDisable(GL_LIGHTING);

    // Draw the horizontal part of the fence as a rectangular bar in white color
  //  glColor3f(0.0f, 0.6f, 0.0f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(length, thickness, thickness);
    glutSolidCube(1.0f);
    glPopMatrix();
    glEnable(GL_LIGHTING);

}
void drawFenceTriangle(float x, float y, float z, float width, float height) {
    // Draw triangular parts of the fence
    //glColor3f(0.0, 0.5f, 0.1f); // Brown color for wood

    glPushMatrix();
    glTranslatef(x, y + height / 2, z);

    glBegin(GL_TRIANGLES);

    // Triangle 1
    glVertex3f(-width / 3, -height / 3, 0.0f);
    glVertex3f(width / 3, -height / 3, 0.0f);
    glVertex3f(0.0f, height / 3, 0.0f);
    glColor3f(0.0, 0.5f, 0.1f); // Brown color for wood

    //     Triangle 2 (Optional)
    glVertex3f(-width / 2, -height / 2, 0.0f);
    glVertex3f(width / 2, -height / 2, 0.0f);
    glVertex3f(0.0f, height / 2, 0.0f);

    glEnd();

    glPopMatrix();
}


void drawFence(float startX, float endX, float y, float z, float height, float barThickness) {
    float fenceWidth = 0.1f;
    float x = startX;
    glColor3f(fenceColor[0], fenceColor[1], fenceColor[2]);
    do {
        drawFenceTriangle(x, y, z, 0.1f, height);
        x += 1.0f;
    } while (x < endX);
    glColor3f(fenceColor[0], fenceColor[1], fenceColor[2]);
    // Draw top horizontal bar
    drawHorizontalBar((startX + endX) / 2, y + height, z, endX - startX, barThickness);
    // Draw bottom horizontal bar
    drawHorizontal2Bar((startX + endX) / 2, y + height / 2, z, endX - startX, barThickness);
}

void drawFerrisWheel() {
    // Draw the support beams
    glPushMatrix();
    //glRotatef(treeRotationAngle, 0.0f, 1.0f, 0.0f);
    glColor3f(0.4f, 0.4f, 0.4f); // Gray color
    glPushMatrix();
    glTranslatef(-3.0f, 4.0f, 0.0f); // Adjust the position
    glScalef(0.2f, supportHeight, 0.2f); // Adjust the scale
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-2.5f, 4.0f, 0.0f); // Adjust the position
    glScalef(0.2f, supportHeight, 0.2f); // Adjust the scale
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the Ferris wheel wheel
    glColor3f(0.5f, 0.5f, 0.5f); // Light gray color
    glPushMatrix();
    glTranslatef(-2.75f, 4.0f + supportHeight / 2, 0.0f); // Adjust the position
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(ferrisWheelRotationAngle, 0.0f, 0.0f, 1.0f); // Apply the rotation
    glutSolidTorus(0.15f, wheelRadius, 20, 20); // Create a torus for the wheel
    glPopMatrix();
 
    // Draw passenger compartments
    glColor3f(0.8f, 0.2f, 0.2f); // Red color
    for (int i = 0; i < 8; i++) {
        glPushMatrix();
        float angle = i * 45.0f; // Angle between compartments
        float x = -2.75f + (wheelRadius + compartmentSpacing) * cos(angle * 3.14159265359 / 180.0f);
        float y = 4.0f + supportHeight / 2;
        float z = (wheelRadius + compartmentSpacing) * sin(angle * 3.14159265359 / 180.0f);
        glTranslatef(x, y, z);
        glScalef(compartmentWidth, compartmentHeight, compartmentWidth); // Adjust the scale
        glRotatef(ferrisWheelRotationAngle, 0.0f, 0.0f, 1.0f); // Apply the rotation
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    glPopMatrix();
}
void drawEllipse(float rx, float ry, int num_segments) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(num_segments);
        float x = rx * cosf(theta);
        float y = ry * sinf(theta);
        glVertex2f(x, y);
    }
    glEnd();
}
void drawEllipse(float rx, float ry, int num_segments, float z) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, z); // Center point
    for (int i = 0; i <= num_segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(num_segments);
        float x = rx * cosf(theta);
        float y = ry * sinf(theta);
        glVertex3f(x, y, z);
    }
    glEnd();
}

void drawCustomObject() {
    glPushMatrix();
    glRotatef(treeRotationAngle, 0.0f, 1.0f, 0.0f);
    glColor3f(0.7f, 0.2f, 0.2f);
    glutSolidCube(1.0f);
    // Setup for drawing red cylinders
    glColor3f(0.7f, 0.3f, 0.7f); // Red color for cylinders
    GLUquadricObj* quadratic = gluNewQuadric();
    float cylinderHeight = 2.0f;
    float cylinderRadius = 0.1f;

    // Positions of the cylinders are at the corners of the cube
    float positions[4][2] = {
        {-0.5f, -0.5f},
        {-0.5f, 0.5f},
        {0.5f, -0.5f},
        {0.5f, 0.5f}
    };
    // Draw upright cylinders
    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        glTranslatef(positions[i][0], 0.5f, positions[i][1]);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(quadratic, 0.1f, 0.1f, 2.0f, 32, 32);
        glPopMatrix();
    }
    float roofHeight = cylinderHeight + 0.5f; // Adjust as needed
       glColor3f(0.6f, 0.2f, 0.3f);
       glPushMatrix();
       glTranslatef(0.0f, roofHeight, 0.0f);
       glRotatef(90, 1.0, 0.0, 0.0);
       drawEllipse(1.0f, 1.0f, 32, 0.0f);
       glPopMatrix();

    glPopMatrix();
}

void drawTree() {
    GLUquadricObj* quadratic;
    quadratic = gluNewQuadric();

    glPushMatrix();
    glRotatef(treeRotationAngle, 0.0f, 1.0f, 0.0f);

    // CYLINDER :)
    glPushMatrix();
    glColor3f(0.55f, 0.27f, 0.07f); // Brown color for the tree trunk
    glRotatef(-90, 1.0f, 0.0f, 0.0f); // Rotate the cylinder to stand upright
    gluCylinder(quadratic, 0.1f, 0.0f, 2.0f, 32, 32); // Increase the height to 3.0f
    glPopMatrix();


    // green area (sphere lower)
    glPushMatrix();
    glColor3f(0.0f, 0.5f, 0.0f);
    glTranslatef(0.0f, 1.2f, 0.0f);
    glutSolidSphere(0.6f, 20, 20);
    glPopMatrix();

    // green area(upper sphere)
    glPushMatrix();
    glColor3f(0.0f, 0.6f, 0.0f);
    glTranslatef(0.0f, 2.1f, 0.0f); // Position it above the lower foliage
    glutSolidSphere(0.5f, 20, 20); // Increased upper foliage size
    glPopMatrix();
    glPopMatrix();
}
void drawLampPost() {
    GLUquadricObj* quadratic = gluNewQuadric();


    glPushMatrix();
    glRotatef(treeRotationAngle, 0.0f, 1.0f, 0.0f);
    //sphere part
    glColor3f(0.6f, 0.7f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 0.0f);
    glutSolidSphere(0.1f, 32, 32);
    glPopMatrix();

    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadratic, 0.05f, 0.05f, 2.0f, 32, 32);
    glPopMatrix();

    //top part
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0.0f, 2.1f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.15f, 0.3f, 32, 32);
    glPopMatrix();
    glPopMatrix();

    // Cleanup
    gluDeleteQuadric(quadratic);
}
void drawGoal() {
    if (goalCollected) {
        
        return; // If the goal has been collected, don't draw it
       
    }

    glPushMatrix(); // Save the current state of the matrix stack

    //prim1 box
    glTranslatef(1.0f, 0.5f, 1.0f); // Move to the goal's location
    glRotatef(GoalRotationAngle, 0.0f, 1.0f, 0.0f); // Apply rotation to the entire goal
    glColor3f(0.8f, 0.6f, 0.2f); // Brown color for the chest
    glPushMatrix();
    glScalef(2.0f, 1.0f, 1.0f); // Scale the cube to chest dimensions
    glutSolidCube(0.5f);
    glPopMatrix();
    
   
    // Draw the balls inside the chest
    glColor3f(1.0f, 0.0f, 0.0f); // Color of the balls
    float ballSize = 0.2f; // Size of the balls
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(-0.15f + i * 0.15f, 0.15f, 0.0f); // Position the balls inside the chest
        glutSolidSphere(ballSize, 20, 20); // Draw the ball
        glPopMatrix();
    }

    
    // Draw the goal (small sphere) inside the chest
    glColor3f(0.0f, 1.0f, 0.0f); // Green color for the goal sphere
    glPushMatrix();
    float goalSize = 0.1f; // Size of the goal sphere
    glTranslatef(0.0f, -0.25f + goalSize, 0.0f); // Position the goal sphere inside the chest
    glutSolidSphere(goalSize, 20, 20); // Draw the goal sphere
    glPopMatrix();


    // Draw the chest cover (prim 2)
    glColor3f(0.8f, 0.4f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, -0.25f); // Position relative to the chest
    glScalef(2.0f, 0.1f, 1.0f); // Scale for the lid dimensions
    glutSolidCube(0.5f);
    glPopMatrix();

    // Draw the lock (prim 3)
    glColor3f(0.93f, 0.84f, 0.25f); // Gold color for the lock
    glPushMatrix();
    glTranslatef(0.0f, 0.1f, 0.25f); // Position the lock on the chest
    glutSolidSphere(0.05f, 10, 10); // Draw the lock
    glPopMatrix();

    glPopMatrix(); // Restore the previous state of the matrix stack
}



void print(int x, int y, char* string) {
    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
    }
}

void drawGemstone() {
    glPushMatrix();
  //  glTranslatef(0.0f, 0.0f, -5.0f);
    glColor3f(0.0f, 0.0f, 0.6f);
    glutSolidOctahedron(); //diamond shape
    glPopMatrix();
}


void Display() {
    
    setupCamera();
    setupLights();
   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
         //    thirdPersonCamera.followPlayer(playerPosition);
         
         //glPushMatrix();
         //glTranslated(3.5, 0.1 * thickness, 2.0);  // Adjust the translation values here
         //glColor3f(0.6, 0.7, 1.0);
         //glScaled(1.0 * scaleFactor, thickness * scaleFactor, 1.0 * scaleFactor); // Adjust the scale factor here
         //glutSolidCube(1);
         //glPopMatrix();
         //********
         //drawWall(0.02,7.0);//left
         //glPushMatrix();
         //glRotated(90, 0, 0, 1.0);
         //*******
         //glPushMatrix();
         //glTranslated(3.5, 0.1 * 0.02, 2.0);  // Adjust the translation values for the new wall
         //glColor3f(0.6, 0.7, 1.0);  // Adjust the color for the new wall
         //glScaled(1 * 7.0, 0.02 * 7.0, 7.0);  // Adjust the scale factor for the new wall
         //glColor3f(0.6, 0.7, 1.0);
         //glutSolidCube(1);
         //glPopMatrix();
         //glPushMatrix();
         //glRotated(90, 0, 0, 1.0);
         
         // printf("Direction: (%.2f, %.2f, %.2f)\n", playerX, playerY, playerZ);
         //    if(gameTime==0){
         //
         //    }
         //goal true boolean to checkkk;
         
         
         
         //glPushMatrix();
         //glTranslated(3.5, 0.1 * 0.02, 2.0);  // Adjust the translation values here
         //glColor3f(0.6, 0.7, 1.0);
         //glScaled(1.0 * 7, 0.02 * 7, 1.0 * 7); // Adjust the scale factor here
         //glutSolidCube(1);
         //glPopMatrix(); //weshh
         //glPopMatrix();
         //glPushMatrix();
         //glRotated(-90, 1.0, 0.0, 0.0);
         //glTranslated(0.0, 1.5, 1.5);
         
         
         glColor3f(0.0, 0.9f, 0.4f); // Brown color for wood
         drawWall(0.02, 10.0);// arddd
         glPopMatrix();
         glPushMatrix();
         
         
         // Draw fences around the perimeter of the ground plane
         float groundSize = 10.0f; // Size of the ground plane
         float fenceHeight = 1.0f; // Height of the fence
         float barThickness = 0.1f; // Thickness of the horizontal bars
         
         // Draw left fence
         glPushMatrix();
         drawFence(-groundSize / 2, groundSize / 2, 0.0f, -groundSize / 2, fenceHeight, barThickness);
         glPopMatrix();
         
         // Draw right fence
         glPushMatrix();
         drawFence(-groundSize / 2, groundSize / 2, 0.0f, groundSize / 2, fenceHeight, barThickness);
         glPopMatrix();
  
         // Draw back fence
         glPushMatrix();
         glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
         drawFence(-groundSize / 2, groundSize / 2, 0.0f, -groundSize / 2, fenceHeight, barThickness);
         glPopMatrix();
         
         // Draw front fence
         glPushMatrix();
         glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
         drawFence(-groundSize / 2, groundSize / 2, 0.0f, groundSize / 2, fenceHeight, barThickness);
         glPopMatrix();
         
         glPushMatrix();
         glTranslatef(0.5f, -2.0f, -2.0f);
         drawFerrisWheel(); // Call the function to draw the Ferris wheel
         glPopMatrix();
         
         glPushMatrix();
         glTranslatef(3.5f, 0.6f, -2.0f);
         drawCustomObject();
         glPopMatrix();
   
    
         glPushMatrix();
         glTranslatef(1.0f, 0.0f, -3.5f); // Adjust x, y, and z accordingly
         
         drawPavilion(); // Call your modified drawSwing function with tubular ropes
         glPopMatrix();
         
         ////rightt
         //glPushMatrix();
         //glTranslated(7.0, 3.5, 2.0);  // Adjust the translation values for the new wall
         //glColor3f(0.7, 0.5, 0.3);  // Adjust the color for the new wall
         //glScaled(0.02 * 7.0, 7.0, 7.0);  // Adjust the scale factor for the new wall
         //glColor3f(0.6, 0.7, 1.0);
         //glutSolidCube(1);
         //glPopMatrix();
         glPushMatrix();
         glTranslatef(-4.0f, 0.0f, 3.0f); // Adjust x, y, z to place the tree
         drawTree(); // Draw the tree
         glPopMatrix();
         
         glPushMatrix();
         glTranslatef(4.0f, 0.0f, 3.0f); // Adjust x, y, z to place the tree
         drawLampPost(); // Draw the tree
         glPopMatrix();
        
    
  
    if (!goalCollected) {
        
        glPushMatrix();
        glTranslatef(-5.0f, -0.09f, -3.5f); // Adjust x, y, z to place the tree
        drawGoal();
        glPopMatrix();
    }
    
        
        glTranslatef(playerX, playerY, playerZ); // Adjust the position of the player
        drawPlayer();
        glPopMatrix();
    
    
         if (rotateTree) {
             treeRotationAngle += 0.5f; // Adjust this value for speed
             if (treeRotationAngle > 360.0f) {
                 treeRotationAngle -= 360.0f;
             }
         }
         
         GoalRotationAngle += 0.5f; // Adjust this value for speed
         if (GoalRotationAngle > 360.0f) {
             GoalRotationAngle -= 360.0f;
             
         }
         if(ferrisWheel){
             ferrisWheelRotationAngle += 3.0f; // Increment the angle for rotation
             if (ferrisWheelRotationAngle > 270.0f) {
                 ferrisWheelRotationAngle -= 270.0f; // Keep the angle in the range 0-360
             }
         }
    if (gameTime == 0) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//clearrr the window
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        
        // Set text color
        glColor3f(1.0, 1.0, 1.0); // White color
        
        // Render the text
        if (goalCollected) {
            // Center "You Win!" in the window
            print(300,300,"Congratulations,You Won :))");

        } else {
            print(300,300,"Game Over,You Lost :(");

           // renderBitmapString(350, 300, GLUT_BITMAP_TIMES_ROMAN_24, "Game Over,You Lost :(");
        }
        
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
        
    glFlush();
    glutSwapBuffers();


}




void Keyboard(unsigned char key, int x, int y) {
    float d = 0.01;
    float minX = -2.4; // Define the minimum X boundary based on your fence positions
    float maxX = 2.4;  // Define the maximum X boundary
    float minZ = -2.4; // Define the minimum Z boundary
    float maxZ = 2.4;  // Define the maximum Z boundary
    int channel;
    switch (key) {
     
        case 'b':
            ferrisWheel = true;
            rotateTree = true;
            
            // Play the animation sound effect
             channel = Mix_PlayChannel(-1, anmationSound, 0);
            if (channel == -1) {
                fprintf(stderr, "Failed to play animation sound effect! SDL_mixer Error: %s\n", Mix_GetError());
            }

            glutPostRedisplay();
            break;
               
           case 'n':
               ferrisWheel=false;
               rotateTree = false;
               glutPostRedisplay();
               break;
               
    case 'y'://side view
        thirdPersonCamera = ThirdPersonCamera(
            15.0f, // Camera's X position to the right of the player, beyond the walls
            5.0f,  // Camera's Y position aligned with the player
            0.0f,  // Camera's Z position aligned with the player
            0.0f,  // Looking at the player's X position
            1.0f,  // Looking at the player's Y position
            0.0f,  // Looking at the player's Z position
            0.0f,  // Up vector's X component
            1.0f,  // Up vector's Y component (up is in the positive Y direction)
            0.0f,  // Up vector's Z component
            10.0f         // Distance from the player (this may need to be adjusted)
        );
        glutPostRedisplay();
        break;


    case 't'://front view
        thirdPersonCamera = ThirdPersonCamera(
            0.3f,  // Camera's X position to the right of the player, beyond the walls
            5.0f,  // Camera's Y position aligned with the player
            12.0f, // Camera's Z position aligned with the player
            0.3f,  // Looking at the player's X position
            1.0f,  // Looking at the player's Y position
            1.0f,  // Looking at the player's Z position
            0.0f,  // Up vector's X component
            1.5f,  // Up vector's Y component (up is in the positive Y direction)
            1.0f,  // Up vector's Z component
            1.0f         // Distance from the player (this may need to be adjusted)
        );
        glutPostRedisplay();
        break;
    case 'r'://back to original view
        thirdPersonCamera = ThirdPersonCamera(
            0.3f, 5.5f, 12.0f, // Default camera position and look-at point
            0.0f, 1.5f, 1.0f, // Default up vector
            1.0f             // Default distance from the player
        );
        glutPostRedisplay();
        break;
    case 'e': // Top view
        thirdPersonCamera = ThirdPersonCamera(
            3.5f,
            16.5f,
            2.0f,
            3.5f,
            0.0f,
            1.5f,
            0.0f,
            0.0f,
            -1.0f,
            0.2f   // Decreased distance from the player
        );
        glutPostRedisplay();
        break;



    case 'w':
        // Move the player backward only if within boundaries
        if (playerZ - playerSpeed < 1.90  && playerZ - playerSpeed > 1.20 && playerX < -1.70 && playerX > -2.30) {
            int channel = Mix_PlayChannel(-1, collisionSound, 0);
                if (channel == -1) {
                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                }
        }
        else {
            if (playerZ - playerSpeed < 1.60 && playerZ - playerSpeed > 1.50 && playerX < 2.10 && playerX > 2.00) {
                int channel = Mix_PlayChannel(-1, collisionSound, 0);
                if (channel == -1) {
                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                }
            }
            else {
                
                if (playerZ - playerSpeed < -1.10  && playerX < 1.20 && playerX > -0.10) {
                    int channel = Mix_PlayChannel(-1, collisionSound, 0);
                    if (channel == -1) {
                        fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                    }
                }
                else {
                    if (playerZ - playerSpeed < -0.70 && playerZ - playerSpeed > -1.30 && playerX < 2.10 && playerX > 1.40) {
                        int channel = Mix_PlayChannel(-1, collisionSound, 0);
                        if (channel == -1) {
                            fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                        }
                    }
                    else {
                        if (playerZ - playerSpeed < -0.90 && playerZ - playerSpeed > -1.10 && playerX < -0.90 && playerX > -1.40) {
                            int channel = Mix_PlayChannel(-1, collisionSound, 0);
                            if (channel == -1) {
                                fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                            }
                        }
                        else{
                            if(playerZ - playerSpeed < -0.95 && playerZ - playerSpeed > -1.45 && playerX < -1.65 && playerX > -2.35 && !goalCollected){
                                goalCollected=true;
                                int channel = Mix_PlayChannel(-1, goalSound, 0);
                                if (channel == -1) {
                                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                                }
                            }
                            else {
                                if (playerZ - playerSpeed >= minZ) {
                                    playerAngle = 180.0f; // Face backward
                                    playerZ -= playerSpeed;
                                    
                                }
                               
                            }
                        }
                    }
                }
            }
        }

//            if(playerZ>minZ)
         //                playerZ-=0.01;
                 // Move the player backward only if within boundaries
        break;
    case 's':
        // Move the player forward only if within boundaries
        if (playerZ +playerSpeed < 1.90 && playerZ + playerSpeed > 1.20 && playerX < -1.70 && playerX > -2.30) {
            int channel = Mix_PlayChannel(-1, collisionSound, 0);
                if (channel == -1) {
                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                }
        }
        else {
            if (playerZ + playerSpeed < 1.60 && playerZ + playerSpeed > 1.50 && playerX < 2.10 && playerX > 2.00) {
                int channel = Mix_PlayChannel(-1, collisionSound, 0);
                if (channel == -1) {
                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                }
            }
            else {
                if (playerZ + playerSpeed < -1.10 && playerX < 1.20 && playerX > -0.10) {
                    int channel = Mix_PlayChannel(-1, collisionSound, 0);
                    if (channel == -1) {
                        fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                    }
                }
                else {
                    if (playerZ + playerSpeed < -0.70 && playerZ + playerSpeed > -1.30 && playerX < 2.10 && playerX > 1.40) {
                        int channel = Mix_PlayChannel(-1, collisionSound, 0);
                        if (channel == -1) {
                            fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                        }
                    }
                    else{
                        if(playerZ - playerSpeed < -0.95 && playerZ - playerSpeed > -1.45 && playerX < -1.65 && playerX > -2.35 && !goalCollected){
                            goalCollected=true;
                            int channel = Mix_PlayChannel(-1, goalSound, 0);
                            if (channel == -1) {
                                fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                            }                        }
                        
                        
                        else {
                            if (playerZ + playerSpeed < -0.90 && playerZ + playerSpeed > -1.10 && playerX < -0.90 && playerX > -1.40) {
                                int channel = Mix_PlayChannel(-1, collisionSound, 0);
                                if (channel == -1) {
                                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                                }
                            }
                            else {
                                if (playerZ + playerSpeed <= maxZ) {
                                    playerAngle = 0.0f; // Face forward
                                    playerZ += playerSpeed;
                                }
                            }
                        }
                    }
                }
            }
        }

        break;
    case 'a':
        // Move the player left only if within boundaries
        if (playerX - playerSpeed < -1.70 && playerX - playerSpeed >-2.30 && playerZ < 1.90 && playerZ > 1.20) {
            int channel = Mix_PlayChannel(-1, collisionSound, 0);
                if (channel == -1) {
                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                }
        }
        else {
            if (playerX - playerSpeed < 2.10 && playerX - playerSpeed >2.00 && playerZ < 1.60 && playerZ > 1.50) {
                int channel = Mix_PlayChannel(-1, collisionSound, 0);
                if (channel == -1) {
                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                }
            }
            else {
                if (playerX - playerSpeed < 1.20 && playerX - playerSpeed > -0.10 && playerZ < -1.10) {
                    int channel = Mix_PlayChannel(-1, collisionSound, 0);
                    if (channel == -1) {
                        fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                    }
                }
                
                else {
                    if (playerX - playerSpeed < 2.10 && playerX - playerSpeed > 1.40 && playerZ < -0.70 && playerZ > -1.30) {
                        int channel = Mix_PlayChannel(-1, collisionSound, 0);
                        if (channel == -1) {
                            fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                        }
                    }
                    else{
                        if(playerZ - playerSpeed < -0.95 && playerZ - playerSpeed > -1.45 && playerX < -1.65 && playerX > -2.35 && !goalCollected){
                            goalCollected=true;
                            int channel = Mix_PlayChannel(-1, goalSound, 0);
                            if (channel == -1) {
                                fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                            }                        }
                        
                        
                        else {
                            if (playerX - playerSpeed < -0.90 && playerX - playerSpeed > -1.40 && playerZ < -0.90 && playerZ > -1.10) {
                                int channel = Mix_PlayChannel(-1, collisionSound, 0);
                                if (channel == -1) {
                                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                                }
                            }
                            else {
                                if (playerX - playerSpeed >= minZ) {
                                    playerAngle = 270.0f; // Face left
                                    playerX -= playerSpeed;
                                }
                            }
                        }
                    }
                }
            }
        }

        break;
    case 'd':
        // Move the player right only if within boundaries
        if (playerX + playerSpeed < -1.70 && playerX + playerSpeed >-2.30 && playerZ < 1.90 && playerZ > 1.20) {
            int channel = Mix_PlayChannel(-1, collisionSound, 0);
                if (channel == -1) {
                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                }
        }
        else {
            if (playerX + playerSpeed < 2.10 && playerX + playerSpeed >2.00 && playerZ < 1.60 && playerZ > 1.50) {
                int channel = Mix_PlayChannel(-1, collisionSound, 0);
                if (channel == -1) {
                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                }
            }
            else {
                if (playerX + playerSpeed < 1.20 && playerX + playerSpeed > -0.10 && playerZ < -1.10) {
                    int channel = Mix_PlayChannel(-1, collisionSound, 0);
                    if (channel == -1) {
                        fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                    }
                }
                else {
                    if (playerX + playerSpeed < 2.10 && playerX + playerSpeed > 1.40 && playerZ < -0.70 && playerZ > -1.30) {
                        int channel = Mix_PlayChannel(-1, collisionSound, 0);
                        if (channel == -1) {
                            fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                        }
                    }
                    else{
                        if(playerZ - playerSpeed < -0.90 && playerZ - playerSpeed > -1.40 && playerX < -1.60 && playerX > -2.30 && !goalCollected){
                            goalCollected=true;
                            int channel = Mix_PlayChannel(-1, goalSound, 0);
                            if (channel == -1) {
                                fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                            }                        }
                        else {
                            if (playerX + playerSpeed < -0.90 && playerX + playerSpeed > -1.40 && playerZ < -0.90 && playerZ > -1.10) {
                                int channel = Mix_PlayChannel(-1, collisionSound, 0);
                                if (channel == -1) {
                                    fprintf(stderr, "Failed to play sound effect! Error: %s\n", Mix_GetError());
                                }
                            }
                            else {
                                if (playerX + playerSpeed <= maxX) {
                                    playerAngle = 90.0f; // Face right
                                    playerX += playerSpeed;
                                }
                            }
                        }
                    }
                }
            }
        }

        break;
        //    case 'q':
        //        // Move the player up (if needed)
        //        playerY += playerSpeed;
        //        break;
        //    case 'e':
        //        // Move the player down (if needed)
        //        playerY -= playerSpeed;
        //        break;
    case 'u':
        thirdPersonCamera.moveY(d);
        thirdPersonCamera.look();
        break;
    case 'i':
        thirdPersonCamera.moveY(-d);
        thirdPersonCamera.look();
        break;
    case 'p':
        thirdPersonCamera.moveX(d);
        thirdPersonCamera.look();
        break;
    case 'j':
        thirdPersonCamera.moveX(-d);
        thirdPersonCamera.look();
        break;
    case 'k':
        thirdPersonCamera.moveZ(d);
        thirdPersonCamera.look();
        break;
    case 'l':
        thirdPersonCamera.moveZ(-d);
        thirdPersonCamera.look();
        break;

    case GLUT_KEY_ESCAPE:
        exit(EXIT_SUCCESS);
    }

    glutPostRedisplay();
}
void Special(int key, int x, int y) {
    float a = 5.0;

    switch (key) {
    case GLUT_KEY_UP:
        thirdPersonCamera.rotateX(a);
        thirdPersonCamera.look();

        break;
    case GLUT_KEY_DOWN:
        thirdPersonCamera.rotateX(-a);
        break;
    case GLUT_KEY_LEFT:
        thirdPersonCamera.rotateY(a);
        thirdPersonCamera.look();

        break;
    case GLUT_KEY_RIGHT:
        thirdPersonCamera.rotateY(-a);
        thirdPersonCamera.look();
        break;
    }

    glutPostRedisplay();
}


void Idle() {
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(50, 50);

    glutCreateWindow("Game 2");
    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutSpecialFunc(Special);

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glShadeModel(GL_SMOOTH);
    glutIdleFunc(Idle);
    srand(time(NULL));
    glutTimerFunc(1000, timer, 0); // Start the game timer
    changeFenceColor(0); // change fence color
    
    
    // After SDL and SDL_mixer initialization
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
            return -1;
        }

        // Initialize SDL_mixer
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            fprintf(stderr, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
            return -1;
        }
    
    winningSound=Mix_LoadWAV("/Users/sarahkhaled/Downloads/light-woosh-7119.mp3");
    if (winningSound == NULL) {
        fprintf(stderr, "Failed to load winning sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    } else {
        // Set sound effect volume to maximum
        
        Mix_VolumeChunk(winningSound, MIX_MAX_VOLUME/2);
    }
    
        // Load collision sound effect
        collisionSound = Mix_LoadWAV("/Users/sarahkhaled/Downloads/doorhit-98828.wav");
        if (collisionSound == NULL) {
            fprintf(stderr, "Failed to load collision sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        } else {
            // Set sound effect volume to maximum
            Mix_VolumeChunk(collisionSound, MIX_MAX_VOLUME/8);
        }

    goalSound=Mix_LoadWAV("/Users/sarahkhaled/Downloads/game-bonus-144751.mp3");
    if (goalSound == NULL) {
        fprintf(stderr, "Failed to load animation sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    } else {
        // Set sound effect volume to maximum
        
        Mix_VolumeChunk(goalSound, MIX_MAX_VOLUME/9);
    }
    anmationSound=Mix_LoadWAV("/Users/sarahkhaled/Downloads/light-woosh-7119.mp3");
    if (anmationSound == NULL) {
        fprintf(stderr, "Failed to load animation sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    } else {
        // Set sound effect volume to maximum
        
        Mix_VolumeChunk(anmationSound, MIX_MAX_VOLUME/2);
    }
    // Load and play background music
    Mix_Music *bgMusic = Mix_LoadMUS("/Users/sarahkhaled/Downloads/life-of-a-wandering-wizard.mp3");
    if (bgMusic == NULL) {
        fprintf(stderr, "Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    if (Mix_PlayMusic(bgMusic, -1) < 0) {
        fprintf(stderr, "Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_FreeMusic(bgMusic);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    Mix_VolumeMusic(MIX_MAX_VOLUME / 4);

    if (Mix_PlayMusic(bgMusic, -1) < 0) {
        fprintf(stderr, "Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
    }


    // Existing GLUT main loop call
    glutMainLoop();
    // Mix_FreeChunk(collisionSound);
    // Mix_FreeChunk(anmationSound);
    // Mix_FreeMusic(bgMusic);
    Mix_CloseAudio();
    SDL_Quit();
    return 1;

}
