#include "Camera.h"

Camera::Camera(int width, int height, glm::vec4 eye) {
	Camera::eye = eye;
	width = width;
	height = height;
	//updateCamera(); 
};
Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix()
{
	return this->lookAt(eye, at, up);
}

glm::mat4 Camera::getProjectionMatrix(bool isOrtho)
{
	if (isOrtho) {
		return this->ortho(-scale, scale, -scale, scale, this->zNear, this->zFar);
	}
	else {
		return this->perspective(fov, aspect, this->zNear, this->zFar);
	}
}

glm::mat4 Camera::lookAt(const glm::vec4& eye, const glm::vec4& at, const glm::vec4& up)
{
	// use glm.
	glm::vec3 eye_3 = eye;
	glm::vec3 at_3 = at;
	glm::vec3 up_3 = up;

	//glm::mat4 c = glm::lookAt(eye_3, eye_3-at_3, up_3);
	//
	//c[3][0] += move.x;
	//c[3][1] += move.y;
	//c[3][2] += move.z;

	//return c;

	return glm::lookAt(eye_3, eye_3 + at_3, up_3);
}

glm::mat4 Camera::ortho(const GLfloat left, const GLfloat right,
	const GLfloat bottom, const GLfloat top,
	const GLfloat zNear, const GLfloat zFar)
{
	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = 2.0 / (right - left);
	c[1][1] = 2.0 / (top - bottom);
	c[2][2] = -2.0 / (zFar - zNear);
	c[3][3] = 1.0;
	c[0][3] = -(right + left) / (right - left);
	c[1][3] = -(top + bottom) / (top - bottom);
	c[2][3] = -(zFar + zNear) / (zFar - zNear);

	c = glm::transpose(c);
	return c;
}

glm::mat4 Camera::perspective(const GLfloat fovy, const GLfloat aspect,
	const GLfloat zNear, const GLfloat zFar)
{
	return glm::perspective(glm::radians(fovy), (float)aspect, zNear, zFar);

}

glm::mat4 Camera::frustum(const GLfloat left, const GLfloat right,
	const GLfloat bottom, const GLfloat top,
	const GLfloat zNear, const GLfloat zFar)
{
	// 任意视锥体矩阵
	glm::mat4 c = glm::mat4(1.0f);
	c[0][0] = 2.0 * zNear / (right - left);
	c[0][2] = (right + left) / (right - left);
	c[1][1] = 2.0 * zNear / (top - bottom);
	c[1][2] = (top + bottom) / (top - bottom);
	c[2][2] = -(zFar + zNear) / (zFar - zNear);
	c[2][3] = -2.0 * zFar * zNear / (zFar - zNear);
	c[3][2] = -1.0;
	c[3][3] = 0.0;

	c = glm::transpose(c);
	return c;
}

void Camera::cameraLimiter() {
	if (cameraNumber == 1) {
		if (eye.x > 4.0779) {
			eye.x = 4.0779;
		}
		else if (eye.x < -4.0779) {
			eye.x = -4.0779;
		}

		if (eye.y > 5 && cameraNumber == 1) {
			eye.y = 5;
		}
		else if (eye.y < -0.9484) {
			eye.y = -0.9484;
		}

		if (eye.z > 4.027) {
			eye.z = 4.027;
		}
		else if (eye.z < -4.027) {
			eye.z = -4.027;
		}
	}
}

//事件监听
void Camera::Inputs(GLFWwindow* window)
{	//camera number：Used to switch camera viewing angle
	if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS) {
		at = recordAt;
		eye = recordEye;
		fixed = false;
		cameraNumber = 1;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS) {
		cameraNumber = 0;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS) {
		eye = glm::vec4(0.035325f, 5.739996f, -0.186067f, 1.0f);
		at = glm::vec4(0.000059f, -0.999792f, 0.020360f, 1.0f);
		fixed = true;
		cameraNumber = 2;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS) {
		eye = glm::vec4(0.0156f, 1.2678f, -4.027f, 1.0f);
		at = glm::vec4(0.00582f, -0.002909f, 0.9998f, 1.0f);
		cameraNumber = 1;//与自由相机一致，可以自由移动，此相机只是帮助定位
		fixed = false;//不限制相机旋转
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS) {//npc头上视角
		eye = glm::vec4(1.673674f, 0.042431f, 2.895273f, 1.0f);
		at = glm::vec4(-0.447943f, -0.121845f, -0.885487f, 1.0f);
		cameraNumber = 1;
		fixed = false;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS) {//人偶脸上视角
		eye = glm::vec4(-0.366f, 0.23f, 2.649f, 1.0f);
		at = glm::vec4(0.13f, -0.2504f, -0.959f, 1.0f);
		cameraNumber = 1;
		fixed = false;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS) {//暗恋视角
		eye = glm::vec4(4.206998f, 4.254737f, 4.159173f, 1.0f);
		at = glm::vec4(-0.685588f, -0.653728f, -0.319692f, 1.0f);
		cameraNumber = 6;
		cameraLimit = false;
		fixed = false;
	}

	//相机视域范围
	if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS) {
		fov = 89.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS) {
		fov = 45.0f;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS) {
		fov = 5.0f;
	}

	if (cameraNumber == 0) eye = glm::vec4(birdLoction, 1.0f);

	if (cameraNumber==1) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {//前
			eye += speed * at;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {//左
			eye += glm::vec4(speed * -glm::normalize(glm::cross((glm::vec3)at, (glm::vec3)up)), 1.0);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {//后
			eye -= speed * at;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {//右
			eye += glm::vec4(speed * glm::normalize(glm::cross((glm::vec3)at, (glm::vec3)up)), 1.0);
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {//上
			eye += speed * up;
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {//下
			eye += speed * -up;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_PRESS) {
			//init
			fov = 90.0f;
			eye = glm::vec4(0.0, -0.5, -2.0, 1.0);
			at = glm::vec4(0.0, 0.0, 1.0, 1.0);
			up = glm::vec4(0.0, 1.0, 0.0, 1.0);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			speed = 0.1f;
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
			speed = 0.04f;
		}

		if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
			cameraLimit = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
			cameraLimit = false;
		}
	}

	if (!fixed) {
		//mouse
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_PRESS) {
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

				if (firstClick) {
					glfwSetCursorPos(window, (width / 2), (height / 2));
					firstClick = false;
				}
				double posx;
				double posy;
				glfwGetCursorPos(window, &posx, &posy);
				//获得光标偏移量
				float offsetx = sensitivity * (float)(posy - (height / 2)) / height;
				float offsety = sensitivity * (float)(posx - (height / 2)) / height;

				glm::vec3 at_3 = glm::vec3(at.x, at.y, at.z);
				glm::vec3 up_3 = glm::vec3(up.x, up.y, up.z);
				//绕Y轴旋转相机，并限制其每次旋转角度
				glm::vec3 newAt = glm::rotate(at_3, glm::radians(-offsetx), glm::normalize(glm::cross(at_3, up_3)));
				if (!glm::angle(newAt, up_3) <= glm::radians(5.0f) || glm::angle(newAt, -up_3) <= glm::radians(5.0f)) {
					//限制相机仰角略微小于90°，俯角略微大于-90°
					if (newAt.y >= -0.998 && newAt.y <= 0.998) {
						at = glm::vec4(newAt, 1.0);
					}
				}
				//绕X轴旋转相机
				newAt = glm::rotate(glm::vec3(at), glm::radians(-offsety), up_3);
					at = glm::vec4(newAt, 1.0);

				glfwSetCursorPos(window, (width / 2), (height / 2));
			}

			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				firstClick = true;
			}
		}
		if (cameraNumber == 2) fov = 89.0f;
		if (!fixed) {
			recordAt = at;
			recordEye = eye;
			if(cameraLimit) cameraLimiter();
		}
	}
}
