#pragma once


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include "stb_image.h"

#include "camera.h"
#include "resourceManager/shader.h"
#include "resourceManager/resource_manager.h"
#include "resourceManager/texture.h"

using namespace std;

namespace Snow {
#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
	//最大速度
#define MAX_VELOC glm::vec3(0.0,-50.0,0.0)
//最小速度
#define MIN_VELOC glm::vec3(0.0,-1.0,0.0)
//发射粒子最大周期
#define MAX_LAUNCH 1.0f*1000.0f
//发射粒子最小周期
#define MIN_LAUNCH 0.3f*1000.0f
//初始点精灵大小
#define INIT_SIZE 10.0f
#define MAX_SIZE 10.0f
#define MIN_SIZE 3.0f

	const int MAX_PARTICLES = 5000;//定义粒子发射系统最大的粒子数
								   //初始发射器例子数量
	const int INIT_PARTICLES = 500;
	//落雪中心
	const glm::vec3 center(0.0f);
	const float areaLength = 150.0f;
	const float fallHeight = 90.0f;

	struct SnowParticle
	{
		float type;
		glm::vec3 position;
		glm::vec3 velocity;
		float lifetimeMills;//年龄
		float size;//粒子点精灵大小
	};

	class CTexture {
	public:
		GLuint textureID;

		CTexture() {

		}

		~CTexture() {
			glDeleteTextures(1, &textureID);
		}

		void loadTexture(string path, bool constrain = false) {
			glGenTextures(1, &textureID);
			int width, height, nrComponents;
			unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);

			if (data) {
				GLenum format;
				switch (nrComponents) {
				case 1:format = GL_RED; break;
				case 3:format = GL_RGB; break;
				case 4:format = GL_RGBA; break;
				}
				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
				if (!constrain) {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
				else {
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				}
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else {
				std::cout << "Texture failed to load at path: " << path << std::endl;
			}
			stbi_image_free(data);
		}
	};

	class Snow
	{
	public:
		Snow()
		{
			mCurVBOIndex = 0;
			mCurTransformFeedbackIndex = 1;
			mFirst = true;
			mTimer = 0;
			const GLchar* varyings[5] = { "Type1","Position1",
				"Velocity1","Age1","Size1"
			};//设置TransformFeedback要捕获的输出变量
			mUpdateShader = new Shader("shaders/snow/vs_snow.glsl", "shaders/snow/fs_snow.glsl",
				"shaders/snow/gs_snow.glsl", varyings, 5);
			//设置TransformFeedback缓存能够记录的顶点的数据类型

			mRenderShader = new Shader("shaders/snow/vs_render.glsl",
				"shaders/snow/fs_render.glsl");
			//设置随机纹理
			InitRandomTexture(512);
			mSparkTexture.loadTexture("res/images/snow/snowstorm.bmp");
			mRenderShader->use();
			mRenderShader->SetInteger("snowflower", 0);
			glUseProgram(10);
			InitSnow();
		}


		~Snow()
		{
		}

		bool InitSnow()
		{
			SnowParticle particles[MAX_PARTICLES];
			memset(particles, 0, sizeof(particles));
			GenInitLocation(particles, INIT_PARTICLES);
			glGenTransformFeedbacks(2, mTransformFeedbacks);
			glGenBuffers(2, mParticleBuffers);
			glGenVertexArrays(2, mParticleArrays);
			for (int i = 0; i < 2; i++)
			{
				glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbacks[i]);
				glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[i]);
				glBindVertexArray(mParticleArrays[i]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBuffers[i]);
				//glBindBuffer(GL_ARRAY_BUFFER,0);
			}
			//glBindTransformFeedback(GL_TRANSFORM_FEEDBACK,0);
			glBindVertexArray(0);
			//绑定纹理
			mUpdateShader->use();
			mUpdateShader->SetInteger("gRandomTexture", 0);
			mUpdateShader->SetFloat("MAX_SIZE", MAX_SIZE);
			mUpdateShader->SetFloat("MIN_SIZE", MIN_SIZE);
			mUpdateShader->SetFloat("MAX_LAUNCH", MAX_LAUNCH);
			mUpdateShader->SetFloat("MIN_LAUNCH", MIN_LAUNCH);
			glUseProgram(0);
			return true;
		}

		void Render(float frametimeMills, glm::mat4& worldMatrix,
			glm::mat4 viewMatrix, glm::mat4& projectMatrix)
		{
			mTimer += frametimeMills * 1000.0f;
			UpdateParticles(frametimeMills * 1000.0f);
			RenderParticles(worldMatrix, viewMatrix, projectMatrix);
			mCurVBOIndex = mCurTransformFeedbackIndex;
			mCurTransformFeedbackIndex = (mCurTransformFeedbackIndex + 1) & 0x1;
		}

		void UpdateParticles(float frametimeMills)
		{
			mUpdateShader->use();
			mUpdateShader->SetFloat("gDeltaTimeMillis", frametimeMills);
			mUpdateShader->SetFloat("gTime", mTimer);
			//绑定纹理
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, mRandomTexture);

			glEnable(GL_RASTERIZER_DISCARD);//我们渲染到TransformFeedback缓存中去，并不需要光栅化
			glBindVertexArray(mParticleArrays[mCurVBOIndex]);
			glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[mCurVBOIndex]);
			glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbacks[mCurTransformFeedbackIndex]);

			glEnableVertexAttribArray(0);//type
			glEnableVertexAttribArray(1);//position
			glEnableVertexAttribArray(2);//velocity
			glEnableVertexAttribArray(3);//lifetime
			glEnableVertexAttribArray(4);//size
			glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, type));
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, position));
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, velocity));
			glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, lifetimeMills));
			glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, size));
			glBeginTransformFeedback(GL_POINTS);
			if (mFirst)
			{
				glDrawArrays(GL_POINTS, 0, INIT_PARTICLES);
				mFirst = false;
			}
			else {
				glDrawTransformFeedback(GL_POINTS, mTransformFeedbacks[mCurVBOIndex]);
			}
			glEndTransformFeedback();
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(3);
			glDisableVertexAttribArray(4);
			glDisable(GL_RASTERIZER_DISCARD);
			//glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void RenderParticles(glm::mat4& worldMatrix,
			glm::mat4& viewMatrix, glm::mat4& projectMatrix)
		{
			glEnable(GL_POINT_SPRITE);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ZERO);

			mRenderShader->use();
			mRenderShader->SetMatrix4("model", worldMatrix);
			mRenderShader->SetMatrix4("view", viewMatrix);
			mRenderShader->SetMatrix4("projection", projectMatrix);
			//glBindVertexArray(mParticleArrays[mCurTransformFeedbackIndex]);
			//glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,mParticleBuffers[mCurTransformFeedbackIndex]);
			glBindBuffer(GL_ARRAY_BUFFER, mParticleBuffers[mCurTransformFeedbackIndex]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, position));
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(SnowParticle), (void*)offsetof(SnowParticle, size));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mSparkTexture.textureID);
			glDrawTransformFeedback(GL_POINTS, mTransformFeedbacks[mCurTransformFeedbackIndex]);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glEnable(GL_DEPTH_TEST);
		}

		void InitRandomTexture(unsigned int size)
		{
			srand(time(NULL));
			glm::vec3* pRandomData = new glm::vec3[size];
			for (int i = 0; i < size; i++)
			{
				pRandomData[i].x = float(rand()) / float(RAND_MAX);
				pRandomData[i].y = float(rand()) / float(RAND_MAX);
				pRandomData[i].z = float(rand()) / float(RAND_MAX);
			}
			glGenTextures(1, &mRandomTexture);
			glBindTexture(GL_TEXTURE_1D, mRandomTexture);
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, size, 0, GL_RGB, GL_FLOAT, pRandomData);
			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			delete[] pRandomData;
			pRandomData = nullptr;
		}

		void GenInitLocation(SnowParticle particles[], int nums)
		{
			srand(time(NULL));
			for (int x = 0; x < nums; x++) {
				glm::vec3 record(0.0f);
				record.x = (2.0f * float(rand()) / float(RAND_MAX) - 1.0f) * areaLength;
				record.z = (2.0f * float(rand()) / float(RAND_MAX) - 1.0f) * areaLength;
				record.y = fallHeight - 10.0f;
				particles[x].type = PARTICLE_TYPE_LAUNCHER;
				particles[x].position = record;
				particles[x].velocity = (MAX_VELOC - MIN_VELOC) * (float(rand()) / float(RAND_MAX))
					+ MIN_VELOC;//在最大最小速度之间随机选择
				particles[x].size = INIT_SIZE;//发射器粒子大小
				particles[x].lifetimeMills = 0.5f * (float(rand()) / float(RAND_MAX)) + 0.1f;
			}
		}

		unsigned int mCurVBOIndex, mCurTransformFeedbackIndex;
		GLuint mParticleBuffers[2]; //粒子发射系统的两个顶点缓存区
		GLuint mParticleArrays[2];
		GLuint mTransformFeedbacks[2];//粒子发射系统对应的TransformFeedback
		GLuint mRandomTexture;//随机一维纹理
		CTexture mSparkTexture;//Alpha纹理
		CTexture mStartTexture;
		float mTimer;//粒子发射器已经发射的时间
		bool mFirst;
		Shader* mUpdateShader;//更新粒子的GPUProgram
		Shader* mRenderShader;//渲染粒子的GPUProgram

	};
}