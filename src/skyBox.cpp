#include "skyBox.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

void SkyBox::drawSkyBox(glm::mat4 camera_view) 
{
	glDepthFunc(GL_LEQUAL);

	m_Shader.Use();
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)1280 / (float)720, 0.0f, 1000.0f);
	m_Shader.SetMatrix4("view", camera_view, true);
	m_Shader.SetMatrix4("projection", projection, true);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeTextureId);

	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
}
