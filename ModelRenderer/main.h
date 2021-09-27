#ifndef _MAIN_H_
#define _MAIN_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
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

#define ENV_NUM 100

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
bool saveScreenshot(std::string filename, int width, int height);

std::vector<std::array<float, 2>> load2Params(const char* filename, int rows);
std::vector<std::array<float, 2>> view_angles;
std::vector<std::array<float, 5>> load5Params(const char* filename, int rows);
std::vector<std::array<float, 5>> params;

GLFWwindow*		initGL();

std::string model_name = "porsche-911-turbo";

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 1.0f));
const float camera_dist = 0.5f;

std::string param_path = "D:/Data/param/input/" + model_name + ".bin";
const int param_row = 1000;
std::string camera_path = "D:/Data/param/input/" + model_name + "_camera_dist.bin";
std::string model_path = "D:/Data/models/vehicle/" + model_name + ".obj";

std::string env_path = "D:/Data/env/";
std::string env_filename = "pink_sunrise_4k.hdr";

std::string save_path = "D:/Data/img/vehicle/" + model_name + "/sphere/";

class ModelRenderer
{
public:
	ModelRenderer(GLFWwindow* window, Camera* _camera);

	void loadShaders();
	void createMaps(std::string env_path);
	void run(GLFWwindow* _window);
	void save(GLFWwindow* _window, std::string _path);

	void setPBRShader();

	// camera
	Camera* pCamera;
	// Material
	Material* pMaterial;

private:
	GLFWwindow* pWindow;
	
	Shader* pPBRShader;
	Cubemap* pCubemap;
	Irradiancemap* pIrradiancemap;
	Prefilteredmap* pPrefilteredmap;
	BRDFmap* pBRDFmap;
	Shader* pBackgroundShader;

	// sphere initialzied as radius, sectors, stacks
	Sphere* pSphere;
	Cube* pCube;

	// Models
	Model* pModel;
};

#endif