#include "main.h"

int main(int argc, char* argv[])
{
	srand(time(0));
	GLFWwindow* window = initGL();
	ModelRenderer mainRenderer(window, &camera);

	// load parameter file
	view_angles = loadCamParams(camera_path.c_str(), param_row);
	params = loadRenderParams(param_path.c_str(), param_row);

	// Load various shaders
	mainRenderer.loadShaders();

	// main rendering loop
	mainRenderer.run(window);
	//mainRenderer.save(window, save_path);

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

ModelRenderer::ModelRenderer(GLFWwindow* window, Camera* _camera)
{
	pWindow = window;
	pCamera = _camera;
	pNormalCamera = new Camera(glm::vec3(0.0f, 0.0f, 1.0f));

	pSphere = new Sphere(128, 128);
	pCube = new Cube();

	// basic material
	glm::vec3 color(0.9f, 0.9f, 0.9f);
	float metal = 1.00f;
	float rough = 0.1f;
	pMaterial = new Material(color, rough, metal);

	pPBRShader = NULL;
	pCubemap = NULL;
	pIrradiancemap = NULL;
	pPrefilteredmap = NULL;
	pBRDFmap = NULL;
	pBackgroundShader = NULL;

	pModel = new Model(model_path + model_name + ".obj");
	pModel->position = glm::mat4(1.0f);
#if DRAW_MODE == 1 || DRAW_MODE == 4
	std::array<float, 9> stats = readTxtFile(model_path + model_name + ".txt");
	camera_dist = stats[0];
	//pModel->position = glm::rotate(pModel->position, glm::radians(0.0f), glm::vec3(1.0, 0.0, 0.0));
	//pModel->position = glm::translate(pModel->position, glm::vec3(0.0f, -0.1f, 0.0f)); // translate it down so it's at the center of the scene
	//pModel->position = glm::scale(pModel->position, glm::vec3(4.0f));	// it's a bit too big for our scene, so scale it down
	pModel->position = glm::rotate(pModel->position, glm::radians(stats[1]), glm::vec3(stats[2], stats[3], stats[4]));
	pModel->position = glm::translate(pModel->position, glm::vec3(stats[5], stats[6], stats[7])); // translate it down so it's at the center of the scene
	pModel->position = glm::scale(pModel->position, glm::vec3(stats[8]));	// it's a bit too big for our scene, so scale it down
#endif
}

void ModelRenderer::run(GLFWwindow* _window)
{
	createMaps(env_path + env_filename);

	int scrWidth, scrHeight;
	glfwGetFramebufferSize(_window, &scrWidth, &scrHeight);
	glViewport(0, 0, scrWidth, scrHeight);

	pCamera->SetPositionDist(0.0f, 0.0f, 0.0f, camera_dist);

	while (!glfwWindowShouldClose(pWindow))
	{
		// per-frame time logic
		// --------------------
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(pWindow);

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		setPBRShader();

		// bind pre-computed IBL data
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, pIrradiancemap->getID());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, pPrefilteredmap->getID());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pBRDFmap->getID());

		//pSphere->render();
		pPBRShader->setMat4("model", pModel->position);
#if DRAW_MODE == 1 || DRAW_MODE == 4
		pModel->Draw(pPBRShader);
		//pSphere->render();
#elif DRAW_MODE == 2 || DRAW_MODE == 3
		pSphere->render();
#endif

		// skybox
		pBackgroundShader->use();
		glm::mat4 view = pCamera->GetViewMatrix();
		pBackgroundShader->setMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, pCubemap->getID());
		pCube->render();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}
}

void ModelRenderer::save(GLFWwindow* _window, std::string _path)
{
	int cnt = 0;
	// load env map
	std::vector<std::string> env_list;
	std::vector<std::string> env_name;
	int env_count = 0;
	pCubemap->loadEnvfromDirectory(env_path, env_list, env_name, env_count);
	int per_env = (int)(param_row / env_count);

	for (int i = 0; i < env_count; i++)
	{
		createMaps(env_list[i].c_str());

		int scrWidth, scrHeight;
		glfwGetFramebufferSize(_window, &scrWidth, &scrHeight);
		glViewport(0, 0, scrWidth, scrHeight);

		for (int j = 0; j < per_env; j++)
		{
			cnt = i * per_env + j;
			// set camera view
			std::array<float, CAMERA_DIMS> view_angle = view_angles[cnt];
			pCamera->SetPositionDist(view_angle[0], view_angle[1], view_angle[2], camera_dist);
			pNormalCamera->SetPositionDist(pCamera->Yaw, pCamera->Pitch, pCamera->Bank, 1.0f);

			// render
			// ------
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#if DRAW_MODE == 1 || DRAW_MODE == 2 || DRAW_MODE == 4
			std::array<float, RENDER_DIMS> param = params[cnt];
			pMaterial->setColor(glm::vec3(param[0], param[1], param[2]));
			pMaterial->setMetallic(param[3]);
			pMaterial->setRoughness(param[4]);
#endif
			setPBRShader();
			// bind pre-computed IBL data
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, pIrradiancemap->getID());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, pPrefilteredmap->getID());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, pBRDFmap->getID());

			pPBRShader->setMat4("model", pModel->position); 
#if DRAW_MODE == 1 || DRAW_MODE == 4
			pModel->Draw(pPBRShader);
#elif DRAW_MODE == 2 || DRAW_MODE == 3
			pSphere->render();
#endif
			// enumerate the screenshot filename
			std::string path = "IMG";
			std::string number = std::to_string(cnt++);
			std::stringstream ss;
			ss << std::setw(5) << std::setfill('0') << number;
			path = _path + path + ss.str() + ".jpg";

			saveScreenshot(path, 256, 256);

			// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
			// -------------------------------------------------------------------------------
			glfwSwapBuffers(pWindow);
			glfwPollEvents();
		}
	}
}

void ModelRenderer::setPBRShader()
{
	pPBRShader->use();
	pPBRShader->setInt("irradianceMap", 0);
	pPBRShader->setInt("prefilterMap", 1);
	pPBRShader->setInt("brdfLUT", 2);

	// pass projection, view, and model matrices to shader
	glm::mat4 projection = glm::perspective(glm::radians(pCamera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	pPBRShader->setMat4("projection", projection);
	glm::mat4 view = pCamera->GetViewMatrix();
	pPBRShader->setMat4("view", view);
	pNormalCamera->SetPositionDist(pCamera->Yaw, pCamera->Pitch, pCamera->Bank, 1.0f);
	glm::mat4 nor_view = pNormalCamera->GetRUDMatrix();
	pPBRShader->setMat4("normal_view", nor_view);
	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	pPBRShader->setMat4("model", model);

	// camera uniform variable
	pPBRShader->setVec3("camPos", pCamera->getPosition());

	pPBRShader->setVec3("albedo", pMaterial->getColor());
	pPBRShader->setFloat("roughness", pMaterial->getRoughness());
	pPBRShader->setFloat("metallic", pMaterial->getMetallic());
	pPBRShader->setFloat("ao", 1.0f);
}

GLFWwindow* initGL()
{
	glewExperimental = true;

	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
		return NULL;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window;
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Renderer", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to open GLFW window." << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return NULL;
	}
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	// set depth function to less than AND equal for skybox depth trick.
	glDepthFunc(GL_LEQUAL);
	// enable seamless cubemap sampling for lower mip levels in the pre-filter map.
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	return window;
}

void ModelRenderer::loadShaders()
{
	// build and compile our shader zprogram
	// ------------------------------------
#if DRAW_MODE != 4
	pPBRShader = new Shader("./shader_code/pbr.vert", "./shader_code/pbr.frag");
#else
	pPBRShader = new Shader("./shader_code/pbr.vert", "./shader_code/pbrNormal.frag");
#endif
	pCubemap = new Cubemap("./shader_code/cubemap.vert", "./shader_code/cubemap.frag");
	pIrradiancemap = new Irradiancemap("./shader_code/cubemap.vert", "./shader_code/irradiance.frag", pCubemap);
	pPrefilteredmap = new Prefilteredmap("./shader_code/cubemap.vert", "./shader_code/prefilter.frag", pCubemap);
	pBRDFmap = new BRDFmap("./shader_code/brdf.vert", "./shader_code/brdf.frag", pCubemap);
	pBackgroundShader = new Shader("./shader_code/background.vert", "./shader_code/background.frag");

	// background shader
	pBackgroundShader->use();
	pBackgroundShader->setInt("environmentMap", 0);
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	pBackgroundShader->setMat4("projection", projection);
}

void ModelRenderer::createMaps(std::string env_path)
{
	// load environment
	pCubemap->loadHDR(env_path.c_str());
	pCubemap->create();

	// pre-calculate illumination maps
	pIrradiancemap->create();
	pPrefilteredmap->create();
	pBRDFmap->create();
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, (float)deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, (float)deltaTime);
}

// glfw: keyboard callback
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement((float)xoffset, (float)yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}

bool saveScreenshot(std::string filename, int width, int height)
{
	// row alignment
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	int nScrSize = SCR_WIDTH * SCR_HEIGHT * 4;
	int nSize = width * height * 4;
	unsigned char* dataBuffer = (unsigned char*)malloc(nScrSize * sizeof(unsigned char));
	unsigned char* resizedBuffer = (unsigned char*)malloc(nSize * sizeof(unsigned char));
	if (!dataBuffer) {
		std::cout << "saveScreenshot() :: buffer allocation error." << std::endl;
		return false;
	}

	// fetch image from the backbuffer
	glReadPixels((GLint)0, (GLint)0, (GLint)SCR_WIDTH, (GLint)SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);

	stbir_resize(dataBuffer, SCR_WIDTH, SCR_HEIGHT, 0, resizedBuffer, width, height, 0,
		STBIR_TYPE_UINT8, 4, STBIR_ALPHA_CHANNEL_NONE, 0,
		STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT,
		STBIR_COLORSPACE_SRGB, nullptr);
	stbi_flip_vertically_on_write(true);
	stbi_write_jpg(filename.c_str(), width, height, 4, resizedBuffer, 100);

	free(dataBuffer);
	free(resizedBuffer);

	std::cout << "saving screenshot(" << filename << ")\n";
	return true;
}

std::vector<std::array<float, CAMERA_DIMS>> loadCamParams(const char* filename, int rows)
{
	std::ifstream fin;
	fin.open(filename, std::ios::in | std::ios::binary);

	std::vector<std::array<float, CAMERA_DIMS>> samples;
	std::array<float, CAMERA_DIMS> temp;

	float param[CAMERA_DIMS];
	for (int i = 0; i < rows; ++i)
	{
		fin.read((char*)param, sizeof(param));
		for (int j = 0; j < CAMERA_DIMS; ++j)
		{
			temp[j] = param[j];
		}
		samples.push_back(temp);
	}
	fin.close();
	return samples;
}

std::vector<std::array<float, RENDER_DIMS>> loadRenderParams(const char* filename, int rows)
{
	std::ifstream fin;
	fin.open(filename, std::ios::in | std::ios::binary);

	std::vector<std::array<float, RENDER_DIMS>> samples;
	std::array<float, RENDER_DIMS> temp;

	float param[RENDER_DIMS];
	for (int i = 0; i < rows; ++i)
	{
		fin.read((char*)param, sizeof(param));
		for (int j = 0; j < RENDER_DIMS; ++j)
		{
			temp[j] = param[j];
		}
		samples.push_back(temp);
	}
	fin.close();
	return samples;
}


std::array<float, 9> readTxtFile(std::string txtfile)
{
	std::array<float, 9> stats = { 0.0f };
	std::ifstream fin(txtfile.c_str());
	std::string type;
	fin >> type >> stats[0];
	fin >> type >> stats[1] >> stats[2] >> stats[3] >> stats[4];
	fin >> type >> stats[5] >> stats[6] >> stats[7];
	fin >> type >> stats[8];
	return stats;
}