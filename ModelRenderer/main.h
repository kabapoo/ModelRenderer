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

#define DRAW_MODE 1
std::string category1 = "vehicle";
std::string category2 = "car";
std::string model_name = "1995-jaguar-xj12-lwb-x305";

const float scale_value = 0.35f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 1.0f));
#if DRAW_MODE == 1
const float camera_dist = 12.0f;
#elif DRAW_MODE == 2
const float camera_dist = 3.0f;
#elif DRAW_MODE == 3
const float camera_dist = 3.0f;
#endif

const int param_row = 1000;
std::string param_path = "D:/Data/param/input/" + category1 + "/" + category2 + "/" + model_name + ".bin";
std::string camera_path = "D:/Data/param/input/" + category1 + "/" + category2 + "/" + model_name + "_camera_dist.bin";
std::string model_path = "D:/Data/models/" + category1 + "/" + category2 + "/" + model_name + ".obj";

std::string env_path = "D:/Data/env/train/";
std::string env_filename = "pink_sunrise_4k.hdr";

#if DRAW_MODE == 1
std::string save_path = "D:/Data/img/" + category1 + "/" + category2 + "/" + model_name + "/origin/";
#elif DRAW_MODE == 2
std::string save_path = "D:/Data/img/" + category1 + "/" + category2 + "/" + model_name + "/sphere/";
#elif DRAW_MODE == 3
std::string save_path = "D:/Data/img/" + category1 + "/" + category2 + "/" + model_name + "/env/";
#endif

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