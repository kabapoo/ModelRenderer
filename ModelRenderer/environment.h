#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <filesystem>

#include "shader.h"
#include "polygon.h"

class Cubemap
{
private:
    unsigned int id;
    unsigned int hdr;
    unsigned int fbo;
    unsigned int rbo;
    glm::mat4 projection;
    glm::mat4 views[6];
    std::vector<std::string> list;
    
    Cube cube;
    Quad quad;

public:
    Cubemap();
    Cubemap(const char* vert, const char* frag);

    void loadHDR(const char* fname);
    void loadEnvfromDirectory(std::string path, std::vector<std::string>& files, std::vector<std::string>& name, int& count);
    // deprived
    //void loadEnvList(std::string path, std::string listname);
    //void loadHDRfromList(std::string path, int idx);
    void setupMatrices();
    void create();

    unsigned int getID() const { return id; }
    unsigned int getHDR() const { return hdr; }
    unsigned int getFBO() const { return fbo; }
    unsigned int getRBO() const { return rbo; }
    glm::mat4 getProjection() const { return projection; }
    glm::mat4 getViews(int i) const { return views[i]; }

    Shader* pShader;
};

class Irradiancemap
{
private:
    unsigned int id;
    Cubemap* pCubemap;
    Cube cube;

public:
    Irradiancemap(const char* vert, const char* frag, Cubemap* p);
    unsigned int getID() const { return id; }
    void create();
    Shader* pShader;
};

class Prefilteredmap
{
private:
    unsigned int id;
    Cubemap* pCubemap;
    Cube cube;

public:
    Prefilteredmap(const char* vert, const char* frag, Cubemap* p);
    unsigned int getID() const { return id; }
    void create();
    Shader* pShader;
};

class BRDFmap
{
private:
    unsigned int id;
    Cubemap* pCubemap;
    Quad quad;

public:
    BRDFmap(const char* vert, const char* frag, Cubemap* p);
    unsigned int getID() const { return id; }
    void create();
    Shader* pShader;
};

#endif