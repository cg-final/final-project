#include "./resourceManager/resource_manager.h"

class Shadow
{
private:
	

public:
	Shader depthShader;
	unsigned int depthMap;
	unsigned int depthMapFBO;
	unsigned int shadowWidth;
	unsigned int shadowHeight;

	Shadow() {

	}

	void initShadow() {
		this->depthShader = ResourceManager::LoadShader("shaders/lightandshadow/vs_shadowDepth.glsl",
			"shaders/lightandshadow/fs_shadowDepth.glsl", nullptr, "shadowDepthShader");
		this->shadowWidth = 1024;
		this->shadowHeight = 1024;

		// framebuffer
		glGenFramebuffers(1, &depthMapFBO);
		// gen texture
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// set to this to avoid outside in shadow
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		// bind texture to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};