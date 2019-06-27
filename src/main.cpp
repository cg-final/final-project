#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "camera.h"
#include "island.h"
#include "skyBox.h"
#include "water.h"
#include "shadow.h"
#include "snow.h"
#include "character.cpp"
#include "model.h"

#include <iostream>
#include <vector>
#include <string>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <ft2build.h>
#include FT_FREETYPE_H

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const char* glsl_version = "#version 130";

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


void drawQuad();
void renderCube();
void drawDepthShader(glm::mat4 lightSpaceMatrix, Shader& depthShader, vector<Model> v);

Camera camera(glm::vec3(0.0, 10.0, 30.0), glm::vec3(0.0, 1.0, 0.0));

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX, lastY;
bool firstMouse = true;
bool ssnow = false;
bool playerMove = false;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	//glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "final", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CLIP_DISTANCE0);


	Water water;
	water.initWater(1000, 0.0);

	Island island(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	island.initIslandShader("terrain");
	island.vao_vbo_init();

	SkyBox skybox;
	skybox.initSkyBox();
	skybox.loadCubeMap();

	glm::vec3 lightColor(1.0f);
	glm::vec3 lightPos = glm::vec3(-3.0f, 18.0f, -13.0f);
	glm::vec3 lightDir = glm::vec3(-1.0f, -1.0f, 1.0f);
	Shader lightBoxShader = ResourceManager::LoadShader("shaders/lightandshadow/vs_lightBox.glsl", "shaders/lightandshadow/fs_lightBox.glsl", nullptr, "lightBoxShader");

	Shadow shadow;
	shadow.initShadow();
	Shader shadowDebugShader = ResourceManager::LoadShader("shaders/lightandshadow/vs_shadowDebugDepth.glsl", "shaders/lightandshadow/fs_shadowDebugDepth.glsl", nullptr, "shadowDebugDepth");
	shadowDebugShader.Use();
	shadowDebugShader.SetInteger("depthMap", 0);

	Snow::Snow snow;
	character chter;

	Shader modelShader = ResourceManager::LoadShader("shaders/model/vs_model.glsl", "shaders/model/fs_model.glsl", nullptr, "modelShader");
	Model house("res/models/house/house.obj");
	Model tree("res/models/201082874427109/tree.obj");
	Model boy("res/models/cartoon_boy_obj/boy.obj");

	vector<Model> models;
	models.push_back(house);
	models.push_back(tree);
	models.push_back(boy);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);


		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 skyboxView = glm::mat3(view);
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);


		glEnable(GL_CLIP_DISTANCE0);
		glBindFramebuffer(GL_FRAMEBUFFER, water.reflectionBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float cameraHeightY = camera.position.y - water.waterHeight;
		camera.position.y -= 2 * cameraHeightY;
		camera.invertPitch();
		glm::mat4 invertCamView = camera.getViewMatrix();
		glm::mat4 invertCamSkyboxView = glm::mat3(invertCamView);

		skybox.drawSkyBox(invertCamSkyboxView);

		island.DrawRefl(invertCamView, projection, glm::vec4(0, 1, 0, 0));


		camera.position.y += 2 * cameraHeightY;
		camera.invertPitch();

		glBindFramebuffer(GL_FRAMEBUFFER, water.refractionBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		skybox.drawSkyBox(skyboxView);
		island.DrawRefl(view, projection, glm::vec4(0, -1, 0, 0));
		glDisable(GL_CLIP_DISTANCE0);

		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 100.0f;
		lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		glViewport(0, 0, shadow.shadowWidth, shadow.shadowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		shadow.depthShader.Use();
		shadow.depthShader.SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);
	

		island.DrawDepthMap(lightSpaceMatrix, shadow.depthShader);
		drawDepthShader(lightSpaceMatrix, shadow.depthShader, models);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		water.drawOcean(view, projection, glfwGetTime(), lightSpaceMatrix, shadow.depthMap, lightDir, lightColor, camera.position);

		island.Draw(view, projection, lightSpaceMatrix, shadow.depthMap, lightDir, lightColor, camera.position);

		skybox.drawSkyBox(skyboxView);

		

		// load model
		modelShader.use();
		glUniform1i(glGetUniformLocation(modelShader.ID, "shadowMap"), 1);

		modelShader.SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);

		/*modelShader.SetVector3f("light.position", lightPos);
		modelShader.SetVector3f("viewPos", camera.position);
		modelShader.SetVector3f("light.color", lightColor);*/

		modelShader.SetVector3f("lightPos", lightPos);
		modelShader.SetVector3f("viewPos", camera.position);
		modelShader.SetBool("shadows", true);
		// light properties
		/*modelShader.SetVector3f("light.ambient", 0.2f, 0.2f, 0.2f);
		modelShader.SetVector3f("light.diffuse", 0.5f, 0.5f, 0.5f);
		modelShader.SetVector3f("light.specular", 1.0f, 1.0f, 1.0f);
		modelShader.SetFloat("light.constant", 1.0f);
		modelShader.SetFloat("light.linear", 0.09f);
		modelShader.SetFloat("light.quadratic", 0.032f);
		// material properties
		modelShader.SetFloat("light.shininess", 32.0f);*/

		// view/projection transformations
		glm::mat4 projection2 = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view2 = camera.getViewMatrix();
		modelShader.SetMatrix4("projection", projection2);
		modelShader.SetMatrix4("view", view2);
		//modelShader.updateView(fov, SRC_WIDTH, SRC_HEIGHT, camera->GetViewMatrix(), false);

		// render the loaded model
		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, glm::vec3(0.0f, 3.8f, 0.0f)); // translate it down so it's at the center of the scene
		model2 = glm::scale(model2, glm::vec3(0.04f, 0.04f, 0.04f));  // it's a bit too big for our scene, so scale it down
		modelShader.SetMatrix4("model", model2);
		house.Draw(modelShader);

		model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, glm::vec3(10.0f, 2.0f, 5.0f)); // translate it down so it's at the center of the scene
		model2 = glm::scale(model2, glm::vec3(0.0015f, 0.0015f, 0.0015f));  // it's a bit too big for our scene, so scale it down
		modelShader.SetMatrix4("model", model2);
		tree.Draw(modelShader);

		model2 = glm::mat4(1.0f);

		if (playerMove) {
			float radius = 10.0f;
			float camX = sin(glfwGetTime()) * radius;
			float camZ = cos(glfwGetTime()) * radius;
			model2 = glm::translate(model2, glm::vec3(camX, 2.0f, camZ));
		}
		else {
			model2 = glm::translate(model2, glm::vec3(20.0f, 1.8f, 0.0f)); // translate it down so it's at the center of the scene

		}
		model2 = glm::scale(model2, glm::vec3(0.03f, 0.03f, 0.03f));  // it's a bit too big for our scene, so scale it down
		modelShader.SetMatrix4("model", model2);
		boy.Draw(modelShader);

		snow.Render(deltaTime, model, view, projection);



		string s = to_string(1.0 / deltaTime);
		s = "fps: " + s;

		//glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		chter.RenderText(s, 50.0f, 670.0f, 0.5f, glm::vec3(0.1f, 0.1f, 0.1f));

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.moveForward(deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.moveRight(deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.moveBack(deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.moveLeft(deltaTime);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		ssnow = true;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		ssnow = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.rotate(yoffset, xoffset);
}

unsigned int quadVAO = 0;
unsigned int quadVBO = 0;
void drawQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right  
			1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left 
			- 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left





		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void drawDepthShader(glm::mat4 lightSpaceMatrix, Shader& depthShader, vector<Model> v) {
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	depthShader.Use();
	depthShader.SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);

	//bamboo
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 3.8f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.04f, 0.04f, 0.04f));    // it's a bit too big for our scene, so scale it down
	depthShader.SetMatrix4("model", model);
	v[0].Draw(depthShader);

	//bamboo2
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(10.0f, 2.0f, 5.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.0015f, 0.0015f, 0.0015f));  // it's a bit too big for our scene, so scale it down
	depthShader.SetMatrix4("model", model);
	v[1].Draw(depthShader);

	//house
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(20.0f, 1.8f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.03f, 0.03f, 0.03f));
	depthShader.SetMatrix4("model", model);
	v[2].Draw(depthShader);
}



