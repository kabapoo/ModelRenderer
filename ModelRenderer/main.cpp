#include "main.h"

int main(int argc, char* argv[])
{
	GLFWwindow* window = initGL();
	ModelRenderer mainRenderer(window, &camera);

	// Load various shaders
	mainRenderer.loadShaders();

	int scrWidth, scrHeight;
	glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
	glViewport(0, 0, scrWidth, scrHeight);

	// main rendering loop
	mainRenderer.run();

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
	
	pSphere = new Sphere(64, 64);
	pCube = new Cube();

	// basic material
	glm::vec3 color(0.1f, 0.12f, 0.4f);
	float metal = 0.00f;
	float rough = 0.15f;
	pMaterial = new Material(color, rough, metal);

	pPBRShader = NULL;
	pCubemap = NULL;
	pIrradiancemap = NULL;
	pPrefilteredmap = NULL;
	pBRDFmap = NULL;
	pBackgroundShader = NULL;

	pModel = new Model("D:/Data/models/Sculpture/Venus Cornell.obj");
}

void ModelRenderer::run()
{
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
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
		pPBRShader->setMat4("model", model);
		pModel->Draw(pPBRShader);

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

void ModelRenderer::setPBRShader()
{
	pPBRShader->use();
	pPBRShader->setInt("irradianceMap", 0);
	pPBRShader->setInt("prefilterMap", 1);
	pPBRShader->setInt("brdfLUT", 2);

	// pass projection, view, and model matrices to shader
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	pPBRShader->setMat4("projection", projection);
	glm::mat4 view = camera.GetViewMatrix();
	pPBRShader->setMat4("view", view);
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
	pPBRShader = new Shader("./shader_code/pbr.vert", "./shader_code/pbr.frag");
	pCubemap = new Cubemap("./shader_code/cubemap.vert", "./shader_code/cubemap.frag");
	pIrradiancemap = new Irradiancemap("./shader_code/cubemap.vert", "./shader_code/irradiance.frag", pCubemap);
	pPrefilteredmap = new Prefilteredmap("./shader_code/cubemap.vert", "./shader_code/prefilter.frag", pCubemap);
	pBRDFmap = new BRDFmap("./shader_code/brdf.vert", "./shader_code/brdf.frag", pCubemap);
	pBackgroundShader = new Shader("./shader_code/background.vert", "./shader_code/background.frag");

	// load environment
	pCubemap->loadHDR(env_filename.c_str());
	pCubemap->create();

	// pre-calculate illumination maps
	pIrradiancemap->create();
	pPrefilteredmap->create();
	pBRDFmap->create();

	// background shader
	pBackgroundShader->use();
	pBackgroundShader->setInt("environmentMap", 0);
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	pBackgroundShader->setMat4("projection", projection);
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
