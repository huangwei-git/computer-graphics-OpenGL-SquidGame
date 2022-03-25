#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Angel.h"
#

class Camera
{
public:
	Camera(int width, int height, glm::vec4 eye);
	~Camera();

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix(bool isOrtho);

	glm::mat4 lookAt(const glm::vec4& eye, const glm::vec4& at, const glm::vec4& up);

	glm::mat4 ortho(const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar);

	glm::mat4 perspective(const GLfloat fov, const GLfloat aspect,
		const GLfloat zNear, const GLfloat zFar);

	glm::mat4 frustum(const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar);

	// 每次更改相机参数后更新一下相关的数值
	//void updateCamera();
	//限制相机 
	void Camera::cameraLimiter();
	void rideBird();
	void freeMode();
	// 处理相机的键盘操作
	void Inputs(GLFWwindow* window);

	// 模视矩阵
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	glm::mat4 projMatrix = glm::mat4(1.0f);

	// 相机位置参数
	float radius = 4.0;
	float rotateAngle = 180.0;
	float upAngle = 0.0;
	glm::vec4 eye;
	glm::vec4 at = glm::vec4(0.0, 0.0, 1.0, 1.0);
	glm::vec4 up = glm::vec4(0.0, 1.0, 0.0, 1.0);

	// 投影参数
	float zNear = 0.1;
	float zFar = 100.0;
	// 透视投影参数
	float fov = 90.0;
	float aspect = 1.0;
	// 正交投影参数
	float scale = 1.5;

	int width = 600;
	int height = 600;
	float speed = 0.000;
	float sensitivity = 100.0f;
	/*
	When judging whether it is the first time to click the screen, 
	the perspective used for division is to move the cursor to the center of the screen.
	*/
	bool firstClick = false;
	bool fixed = false;//Fix the camera and limit its rotation.
	bool cameraLimit = false;//Limits the angle of rotation of the camera.
	glm::vec4 recordEye = glm::vec4(0.0, -0.5, -2.0, 1.0);
	glm::vec4 recordAt = glm::vec4(0.0, 0.0, 1.0, 1.0);
	float upAngleDelta = 0.0;
	float rotateAngleDelta = 0.0;
	int cameraNumber = 1;// 

	glm::vec3 birdLoction = glm::vec3(0.0f,0.0f,0.0f);
	glm::vec3 carLoction2 = glm::vec3(0.0f, 0.0f, 0.0f);
};
#endif