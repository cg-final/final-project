#ifndef island_h
#define island_h
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include "resourceManager/resource_manager.h"
#include "stb_image.h"

using namespace std;

class Island{
public:
    glm::vec3  Position, Size;
    unsigned int VAO, VBO;
    int width, height;
    Shader shader;
    vector<glm::vec3> m_vertexs;
    vector<glm::vec3> m_textures;
    vector<glm::vec3> m_normals;
    vector<glm::vec3> m_total;
	Island() : Position(0, 0, 0), Size(1, 1, 1) { }

	Island(glm::vec3 pos, glm::vec3 size) : Position(pos), Size(size) {
		int nrChannels;
		unsigned char* image = stbi_load("res/images/island.jpg", &width, &height, &nrChannels, 0);
		m_vertexs.clear();
		m_normals.clear();
		m_textures.clear();
		for (int i = 0; i < height - 1; i++)   // 0 1 2 |0| 3 4 5 |1|
		{
			for (int k = 0; k < width - 1; k++)
			{
				// 生成顶点数组, 坐标按照三角网格处理 GL_TRIGANLES
				glm::vec3 v1((k + 0), image[3 * ((i + 0) * width + (k + 0))], (i + 0));
				glm::vec3 v2((k + 0), image[3 * ((i + 1) * width + (k + 0))], (i + 1));
				glm::vec3 v3((k + 1), image[3 * ((i + 1) * width + (k + 1))], (i + 1));
				glm::vec3 v4((k + 1), image[3 * ((i + 0) * width + (k + 1))], (i + 0));
				m_vertexs.push_back(v1);
				m_vertexs.push_back(v2);
				m_vertexs.push_back(v3);
				m_vertexs.push_back(v1);
				m_vertexs.push_back(v3);
				m_vertexs.push_back(v4);

				// normals
				glm::vec3 e1 = v2 - v1;
				glm::vec3 e2 = v3 - v1;
				glm::vec3 n1 = glm::normalize(glm::cross(e1, e2));
				m_normals.push_back(n1);
				m_normals.push_back(n1);
				m_normals.push_back(n1);
				glm::vec3 e3 = v3 - v1;
				glm::vec3 e4 = v4 - v1;
				glm::vec3 n2 = glm::normalize(glm::cross(e3, e4));
				m_normals.push_back(n2);
				m_normals.push_back(n2);
				m_normals.push_back(n2);
			}
		}
		// 生成纹理坐标
		for (int i = 0; i < m_vertexs.size(); i++)
		{
			glm::vec3 tmp(m_vertexs[i].x / width * 10.0f, m_vertexs[i].z / height * 10.0f, 0);
			m_textures.push_back(tmp);
		}
	}

	Shader& initIslandShader(const char* name)
	{
		ResourceManager::LoadShader("shaders/island/vs_island.glsl", "shaders/island/fs_island.glsl", nullptr, name);
		this->shader = ResourceManager::GetShader(name);

		ResourceManager::LoadTexture("res/images/island/sand.jpg", false, "sand");
		ResourceManager::LoadTexture("res/images/island/grass.jpg", false, "grass");
		ResourceManager::LoadTexture("res/images/island/rock.jpeg", false, "snowfield");

		this->shader.Use();
		this->shader.SetInteger("texture0", 6, true);
		this->shader.SetInteger("texture1", 7, true);
		this->shader.SetInteger("texture2", 8, true);
		this->shader.SetInteger("shadowMap", 9, true);

		return this->shader;
	}

	void vao_vbo_init()
	{
		for (int i = 0; i < m_vertexs.size(); i++)
		{
			m_total.push_back(m_vertexs[i]);
			m_total.push_back(m_normals[i]);
			m_total.push_back(m_textures[i]);
		}

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, m_total.size() * sizeof(m_total[0]), &m_total[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(m_total[0]), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(m_total[0]), (void*)(sizeof(m_total[0])));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(m_total[0]), (void*)(2 * sizeof(m_total[0])));
	}

	void DrawRefl(glm::mat4 view, glm::mat4 projection, glm::vec4 oceanPlane)
	{
		shader.Use();
		Texture2D sand = ResourceManager::GetTexture("sand");
		Texture2D grass = ResourceManager::GetTexture("grass");
		Texture2D snowfield = ResourceManager::GetTexture("snowfield");
		glActiveTexture(GL_TEXTURE6);
		sand.Bind();
		glActiveTexture(GL_TEXTURE7);
		grass.Bind();
		glActiveTexture(GL_TEXTURE8);
		snowfield.Bind();

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-width * 0.5f * 0.1f, -1.1f, -height * 0.5f * 0.1f));
		model = glm::scale(model, glm::vec3(0.1f));
		shader.SetMatrix4("model", model);
		shader.SetMatrix4("view", view);
		shader.SetMatrix4("projection", projection);
		shader.SetVector4f("oceanPlane", oceanPlane);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, (int)m_vertexs.size());
		glBindVertexArray(0);
	}

	void DrawDepthMap(glm::mat4 lightSpaceMatrix, Shader& depthShader)
	{
		depthShader.Use();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-width * 0.5f * 0.1f, -1.1f, -height * 0.5f * 0.1f));
		model = glm::scale(model, glm::vec3(0.1f));
		depthShader.SetMatrix4("model", model);
		depthShader.SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, (int)m_vertexs.size());
		glBindVertexArray(0);
	}

	void Draw(glm::mat4 view, glm::mat4 projection, glm::mat4 lightSpaceMatrix, unsigned int shadowMap,
		glm::vec3 lightDir, glm::vec3 lightColor, glm::vec3 viewPos)
	{
		shader.Use();
		Texture2D snowfield = ResourceManager::GetTexture("snowfield");
		Texture2D grass = ResourceManager::GetTexture("grass");
		Texture2D sand = ResourceManager::GetTexture("sand");
		glActiveTexture(GL_TEXTURE6);
		sand.Bind();
		glActiveTexture(GL_TEXTURE7);
		grass.Bind();
		glActiveTexture(GL_TEXTURE8);
		snowfield.Bind();
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, shadowMap);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-width * 0.5f * 0.1f, -1.1f, -height * 0.5f * 0.1f));
		model = glm::scale(model, glm::vec3(0.1f));
		shader.SetMatrix4("model", model);
		shader.SetMatrix4("view", view);
		shader.SetMatrix4("projection", projection);
		shader.SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);
		shader.SetVector3f("lightDir", lightDir);
		shader.SetVector3f("lightColor", lightColor);
		shader.SetVector3f("viewPos", viewPos);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, (int)m_vertexs.size());
		glBindVertexArray(0);
	}
};

#endif
