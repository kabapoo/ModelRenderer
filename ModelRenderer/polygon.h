#ifndef _POLYGON_H_
#define _POLYGON_H_

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

class Sphere
{
private:
    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;

    unsigned int x_segment;
    unsigned int y_segment;
    unsigned int indexCount;

public:
    Sphere(unsigned int x_seg, unsigned int y_seg);

    void render();
};


class Cube
{
private:
    unsigned int vao;
    unsigned int vbo;

public:
    Cube();

    void render();
};


class Quad
{
private:
    unsigned int vao;
    unsigned int vbo;

public:
    Quad();

    void render();
};
#endif