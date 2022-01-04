#ifndef _MAIN_H_
#define _MAIN_H_

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <ctime>
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

#define CAMERA_DIMS 3
#define RENDER_DIMS 5

// enable optimus!
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


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
std::array<float, 9> readTxtFile(std::string txtfile);

std::vector<std::array<float, CAMERA_DIMS>> loadCamParams(const char* filename, int rows);
std::vector<std::array<float, CAMERA_DIMS>> view_angles;
std::vector<std::array<float, RENDER_DIMS>> loadRenderParams(const char* filename, int rows);
std::vector<std::array<float, RENDER_DIMS>> params;

GLFWwindow*		initGL();

#define DRAW_MODE 4
std::string category1 = "vehicles";
//std::string category2 = "..";
std::string model_name = "old_tank";

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
#if DRAW_MODE == 1
float camera_dist = 3.0f;
#elif DRAW_MODE == 2
float camera_dist = 2.626f;
#elif DRAW_MODE == 3
float camera_dist = 2.626f;
#elif DRAW_MODE == 4 
float camera_dist = 4.0f;
#endif

const int param_row = 1000;
std::string param_path = "D:/Data/param/input/" + category1 + "/" + model_name + ".bin";
std::string camera_path = "D:/Data/param/input/" + category1 + "/" + model_name + "_camera_angle.bin";
std::string model_path = "D:/Data/obj/" + category1 + "/";

std::string env_path = "D:/Data/env/mixed/train/";
std::string env_filename = "abandoned_tank_farm_05_2k.hdr";

#if DRAW_MODE == 1
std::string save_path = "D:/Data/img/" + category1 + "/" + model_name + "/origin/";
#elif DRAW_MODE == 2
std::string save_path = "D:/Data/img/" + category1 + "/" + model_name + "/sphere/";
#elif DRAW_MODE == 3
std::string save_path = "D:/Data/img/" + category1 + "/" + model_name + "/env/";
#elif DRAW_MODE == 4
std::string save_path = "D:/Data/img/" + category1 + "/" + model_name + "/normal/";
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

	Camera* pNormalCamera;

	// sphere initialzied as radius, sectors, stacks
	Sphere* pSphere;
	Cube* pCube;

	// Models
	Model* pModel;
};

#endif