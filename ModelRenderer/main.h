#ifndef _MAIN_H_
#define _MAIN_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <assimp/Importer.hpp>

#include "camera.h"
#include "shader.h"
#include "environment.h"
#include "polygon.h"
#include "material.h"
#include "model.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"

#define SCR_WIDTH 640
#define SCR_HEIGHT 640

double lastX = SCR_WIDTH / 2.0;
double lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
double deltaTime = 0.0;	// time between current frame and last frame
double lastFrame = 0.0;

// callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

GLFWwindow*		initGL();

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

class ModelRenderer
{
public:
	ModelRenderer(GLFWwindow* window, Camera* _camera);

	void loadShaders();
	void run();

	void setPBRShader();

private:
	GLFWwindow* pWindow;
	
	Shader* pPBRShader;
	Cubemap* pCubemap;
	Irradiancemap* pIrradiancemap;
	Prefilteredmap* pPrefilteredmap;
	BRDFmap* pBRDFmap;
	Shader* pBackgroundShader;

	// camera
	Camera* pCamera;
	// Material
	Material* pMaterial;

	// sphere initialzied as radius, sectors, stacks
	Sphere* pSphere;
	Cube* pCube;

	// Models
	Model* pModel;

	std::string env_filename = "D:/repos/env/pink_sunrise_4k.hdr";
};

#endif