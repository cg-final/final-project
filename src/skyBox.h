#ifndef skyBox_h
#define skyBox_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "resourceManager/resource_manager.h"
#include "camera.h"
#include "stb_image.h"

using namespace std;

class SkyBox{
public:
	GLuint  m_cubeTextureId;
	Shader  m_Shader;
	GLuint  skyboxVAO;

	SkyBox() :
		m_cubeTextureId(0), skyboxVAO(0) {
		ResourceManager::LoadShader("shaders/skybox/vs_skybox.glsl", "shaders/skybox/fs_skybox.glsl", nullptr, "skybox");
		m_Shader = ResourceManager::GetShader("skybox");
	}
	~SkyBox() {
	}

	void initSkyBox()
	{
		float skyboxVertices[] = {
			// positions
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f,  1.0f
		};
		unsigned int skyboxVBO;
		glGenBuffers(1, &skyboxVBO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glGenVertexArrays(1, &skyboxVAO);
		glBindVertexArray(skyboxVAO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glBindVertexArray(0);
	}

	bool loadCubeMap()
	{
		glGenTextures(1, &m_cubeTextureId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeTextureId);

		std::vector<std::string> images;
		images.push_back("res/images/SkyBox/right.jpg");
		images.push_back("res/images/SkyBox/left.jpg");
		images.push_back("res/images/SkyBox/up.jpg");
		images.push_back("res/images/SkyBox/down.jpg");
		images.push_back("res/images/SkyBox/front.jpg");
		images.push_back("res/images/SkyBox/back.jpg");

		int nW = 0, nH = 0, nrChannels = 0;
		unsigned char* pChImg = nullptr;
		for (int i = 0; i < images.size(); i++)
		{
			//stbi_set_flip_vertically_on_load(false);
			pChImg = stbi_load(images[i].c_str(), &nW, &nH, &nrChannels, 0);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
				nW, nH, 0, GL_RGB, GL_UNSIGNED_BYTE, pChImg);
			stbi_image_free(pChImg);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		m_Shader.Use();
		m_Shader.SetInteger("skybox", 0);

		return true;
	}

	void drawSkyBox(glm::mat4 camera_view);
};

#endif
