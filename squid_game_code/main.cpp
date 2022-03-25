#include "Angel.h"
#include "TriMesh.h"
#include "Camera.h"
#include "MeshPainter.h"
#include "stb_image.h"
//#include "shader.h"

#include <cstdlib>
#include <time.h>
#include <vector>
#include <string>
#include "main.h"

// 窗口大小
int WIDTH = 800;
int HEIGHT = 800;

// 获取帧间隔时间
float deltaTime;
float lastFrame;

int mainWindow;
float shadowLoc = -1.1;// 阴影位置
float gameTime = 18.0f;
float gapTime = 3;
float gameStart;

Camera* camera = new Camera(WIDTH,HEIGHT,glm::vec4(0.0, -0.5, -2.0, 1.0));
Light* light = new Light();
MeshPainter* painter = new MeshPainter();
MeshPainter* painter2 = new MeshPainter();
MeshPainter* painter3 = new MeshPainter();
glm::vec3 recordLightPos;

// 除了当前的玩偶模型外，其他的scale都设置为0
// 为了不用在切换模型时才读取模型数据
TriMesh* currentWawa;// 当前玩偶使用的模型
TriMesh* wawa_black;
TriMesh* wawa_red;
TriMesh* wawa_joker;
TriMesh* wawa_joker_girl;

TriMesh* mesh_coffin;
TriMesh* mesh_npc1;
TriMesh* mesh_npc2;

// 计时灯牌
TriMesh* currentBrand;
TriMesh* brand3;
TriMesh* brand2;
TriMesh* brand1;
TriMesh* brand0;

TriMesh* lighMesh;


TriMesh* body = new TriMesh();
TriMesh* leftWing = new TriMesh();
TriMesh* rightWing = new TriMesh();
//脖子分为多个模型，连接身体的脖子模型为neck1
TriMesh* neck1 = new TriMesh();
TriMesh* neck2 = new TriMesh();
TriMesh* neck3 = new TriMesh();
TriMesh* neck4 = new TriMesh();
TriMesh* hd = new TriMesh();
TriMesh* leftLeg = new TriMesh();
TriMesh* rightLeg = new TriMesh();
TriMesh* leftLowerLeg = new TriMesh();
TriMesh* rightLowerLeg = new TriMesh();
TriMesh* leftFoot = new TriMesh();
TriMesh* rightFoot = new TriMesh();


TriMesh* carBody = new TriMesh();
TriMesh* carFLWheel = new TriMesh();
TriMesh* carBLWheel = new TriMesh();
TriMesh* carFRWheel = new TriMesh();
TriMesh* carBRWheel = new TriMesh();
TriMesh* carLeftLight = new TriMesh();
TriMesh* carRightLight = new TriMesh();
// 这个用来回收和删除我们创建的物体对象
std::vector<TriMesh*> meshList;

class MatrixStack {
	int		_index;
	int		_size;
	glm::mat4* _matrices;

public:
	MatrixStack(int numMatrices = 100) :_index(0), _size(numMatrices)
	{
		_matrices = new glm::mat4[numMatrices];
	}

	~MatrixStack()
	{
		delete[]_matrices;
	}

	void push(const glm::mat4& m) {
		assert(_index + 1 < _size);
		_matrices[_index++] = m;
	}

	glm::mat4& pop() {
		assert(_index - 1 >= 0);
		_index--;
		return _matrices[_index];
	}
};

float skyboxVertices[] = {
	-1.0f,-1.0f,1.0f,
	1.0f,-1.0f,1.0f,
	1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f,1.0f,1.0f,
	1.0f,1.0f,1.0f,
	1.0f,1.0f,-1.0f,
	-1.0f,1.0f,-1.0f
};

void getSize(TriMesh* mesh) {
	std::vector<glm::vec3> pos =  mesh->getVertexPositions();
	//存储模型顶点数据中，沿各个轴方向的最大与最小值
	float max_x = -FLT_MAX;
	float max_y = -FLT_MAX;
	float max_z = -FLT_MAX;
	float min_x = FLT_MAX;
	float min_y = FLT_MAX;
	float min_z = FLT_MAX;
	//遍历顶点
	for (int i = 0; i < pos.size(); i++) {
		auto& position = pos[i];
		if (position.x > max_x) max_x = position.x;
		if (position.y > max_y) max_y = position.y;
		if (position.z > max_z) max_z = position.z;
		if (position.x < min_x) min_x = position.x;
		if (position.y < min_y) min_y = position.y;
		if (position.z < min_z) min_z = position.z;
	}
	//计算尺寸并存储在TriMesh中
	mesh->setWidth(max_x - min_x);
	mesh->setHeight(max_y - min_y);
	mesh->setLength(max_z - min_z);
}

struct Car {
	bool shadow = true;
	float frontSpeed = 0.012f;
	float backSpeed = 0.008f;
	float rotateZ = 20.0f;
	float speed = 5.0f;
	float rotateWheelLimit = 30.0f;
	float rotateWheel = 5.0f;
	float rotateCarBody = 2.5f;
	float cameraLoc = -1;

	float inten = 0.007f;
	float carBodyLength = 180.0f * inten;
	float carBodyWidth = 150.0f * inten;
	float carBodyHeight = 170.0f * inten;

	float wheelLength = 50.0f * inten;
	float wheelWidth = 40.0f * inten;
	float wheelHeight = 50.0f * inten;

	float lightLength = 1 * inten;
	float lightWidth = 10 * inten;
	float lightHeight = 7 * inten;

	// 关节角和菜单选项值
	enum {
		CarBody,
		WheelY,
		WheelZ
	};

	// 关节角大小
	GLfloat theta[3] = {
		0.0,    // Body
		0.0,
		0.0
	};

	bool front = false;;
	bool back = false;
	bool left = false;
	bool right = false;
};

//Hierarchical modeling: ostrich
struct Ostrich
{
	bool shadow = true;

	float inten = 0.005;
	// 关节大小
	float bodyLength = 110.0f * inten;
	float bodyWidth = 130.0f * inten;
	float bodyHeight = 95.0f * inten;

	float wingLength = 60.0f * inten;
	float wingWidth = 20.0f * inten;
	float wingHeight = 50.0f * inten;

	float legWidth = 35.0f * inten;
	float legHeight = 55.0f * inten;
	float legLength = 35.0f * inten;

	float lowerLegWidth = 33.0 * inten;
	float lowerLegHeight = 45.0f * inten;
	float lowerLegLength = 33.0f * inten;

	float footWidth = 34.0 * inten;
	float footHeight = 56.0f * inten;
	float footLength = 48.0f * inten;

	float neck1Width = 30.0f * inten;
	float neck1Height = 30.0f * inten;
	float neck1Length = 30.0f * inten;

	float neck2Width = 20.0f * inten;
	float neck2Height = 50.0f * inten;
	float neck2Length = 20.0f * inten;

	float neck3Width = 20.0f * inten;
	float neck3Height = 50.0f * inten;
	float neck3Length = 20.0f * inten;

	float neck4Width = 20.0f * inten;
	float neck4Height = 25.0f * inten;
	float neck4Length = 20.0f * inten;

	float headLength = 30.0f * inten;
	float headWidth = 20.0f * inten;
	float headHeight = 24.0f * inten;


	// 关节角和菜单选项值
	enum {
		Body,			// 
		Head,			// 
		Head2,
		LeftWing,	// 
		RightWing,	//
		Neck1,	// 
		Neck2,	// 
		Neck3,	// 
		LeftLeg,	// 
		RightLeg,	// 
		LeftLowerLeg,
		RightLowerLeg,
		LeftFoot,
		RightFoot
	};

	// 关节角大小
	GLfloat theta[14] = {
		0.0,    // Body
		-40.0,    // Head
		0.0,		//Head2
		0.0,    // LeftWing
		0.0,    // RightWing
		-20.0,    // NeckLower
		15.0,    // NeckMiddle
		50.0,    // NeckUpper
		55.0,    // LeftLeg
		55.0,     // RightLeg
		270.0,	//LeftLowerLeg
		270.0,	//RightLowerLeg
		35.0,	//LeftFoot
		35.0		//RightFoot
	};

	float speed = 5;
	float rotateSpeed = 1.25f;
	float walkSpeed = 0.008;

	bool action1 = false;
	//move1
	int move1state = 0;
	float wingDire = 1;
	float wingMax = 80;
	float wingMin = 0;
	float wingSpeed = (wingMax - wingMin)/(speed*2);

	//move2
	int move2state = 0;
	bool isLeft2 = true;
	float d2_1 = -1;
	//float legMax2 = 55;
	//float legMin2 = 5;
	float legMax2 = 55;
	float legMin2 = -20;
	float Legspeed_2 = (legMax2 - legMin2) / speed;
	float d2_2 = -1;
	//float lowerLegMax2 = 270;
	//float lowerLegMin2 = 220;
	float lowerLegMax2 = 270;
	float lowerLegMin2 = 255;
	float lowerLegSpeed = (lowerLegMax2 - lowerLegMin2) / speed;
	float d2_3 = 1;
	//float footMax2 = 130;
	//float footMin2 = 35;
	float footMax2 = 160;
	float footMin2 = 35;
	float footSpeed = (footMax2 - footMin2) / speed;
	
	//move3
	float inten3 = 2;
	int move3state = 0;
	float neck1Max3 = 40;
	float neck1Min3 = -20;
	float neck1Speed3 = (neck1Max3 - neck1Min3)/(speed*inten3);
	float d3_1 = 1;
	float neck2Max3 = 25;
	float neck2Min3 = 15;
	float neck2Speed3 = (neck2Max3 - neck2Min3)/ (speed * inten3);
	float d3_2 = 1;
	float neck3Max3 = 50;
	float neck3Min3 = 25;
	float neck3Speed3 = (neck3Max3 - neck3Min3) / (speed * inten3);
	float d3_3 = -1;
	float headMax3 = -40;
	float headMin3 = -75;
	float headSpeed3 = (neck2Max3 - neck2Min3) / (speed * inten3);
	float d3_4 = -1;

	bool action2 = false;
	//move4
	float inten4 = 2;
	int move4state = 0;
	float neck1Max4 = 20;
	float neck1Min4 = -20;
	float neck1Speed4 = (neck1Max4 - neck1Min4) / (speed * inten4);
	float d4_1 = 1;
	float neck2Max4 = 40;
	float neck2Min4 = 15;
	float neck2Speed4 = (neck2Max4 - neck2Min4) / (speed * inten4);
	float d4_2 = 1;
	float neck3Max4 = 50;
	float neck3Min4 = 20;
	float neck3Speed4 = (neck3Max4 - neck3Min4) / (speed * inten4);
	float d4_3 = -1;
	float headMax4 = -40;
	float headMin4 = -70;
	float headSpeed4 = (neck2Max4 - neck2Min4) / (speed * inten4);
	float d4_4 = -1;
	
	//move5
	float inten5 = 2;
	int move5state = 0;
	float loading = 0.0;
	float loadSpeed = 0.005;
	float d5 = 1;
	float headMax5 = 65.0f;
	float headMin5 = -65.0f;
	float headSpeed5 = (headMax5 / (speed * inten5));
};
Ostrich ostrich;
Car car;
bool modelIsOstrich = true;// 当前控制模型是鸵鸟

void MeshTranslation(TriMesh* mesh, glm::mat4 modelMatrix,
	float a, float b, float c,  //部位移动信息 
	float width, float height, float length) { //部位尺寸信息
	// 本节点局部变换矩阵
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(a, b, c));
	instance = glm::scale(instance, glm::vec3(width, height, length));
	mesh->robot = true;		//标记为机器人
	mesh->modelMatrix = modelMatrix * instance;
}

float transX = -0.111f;
float transY = -0.73f;
float transZ = -3.466f;
void getRobotModelMatrix() {
	// 物体的变换矩阵
	glm::mat4 modelMatrix = glm::mat4(1.0);

	// 保持变换矩阵的栈
	MatrixStack mstack;

	// 躯干
	modelMatrix = glm::translate(modelMatrix, glm::vec3(transX, transY,transZ));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(glm::vec3(90.0f, 0.0f, 0.0f).x), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.Body]), glm::vec3(0.0, 1.0, 0.0));
	MeshTranslation(body, modelMatrix, 0.0f, 0.5 * ostrich.bodyHeight * body->getHeight(), 0.0f, ostrich.bodyLength, ostrich.bodyHeight, ostrich.bodyWidth);
	mstack.push(modelMatrix);

		//左腿
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.25f*ostrich.bodyLength * body->getWidth(), 0.0f, -0.35 * ostrich.bodyWidth * body->getLength() + 0.5 * ostrich.legWidth * leftLeg->getLength()));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.LeftLeg]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(leftLeg, modelMatrix, 0.0f, -0.5 * ostrich.legHeight * leftLeg->getHeight() , 0.0f, ostrich.legLength, ostrich.legHeight, ostrich.legWidth);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -ostrich.legHeight * leftLeg->getHeight(), 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.LeftLowerLeg]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(leftLowerLeg, modelMatrix, 0.0f, -0.5 * ostrich.lowerLegHeight * leftLowerLeg->getHeight(), 0.0f, ostrich.lowerLegLength, ostrich.lowerLegHeight, ostrich.lowerLegWidth);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5 * ostrich.lowerLegLength * leftLowerLeg->getWidth(), -ostrich.lowerLegHeight * leftLowerLeg->getHeight(), 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.LeftFoot]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(leftFoot, modelMatrix, -0.5f * ostrich.footLength * leftFoot->getWidth(), -0.5 * ostrich.footHeight * leftFoot->getHeight(), 0.0f, ostrich.footLength, ostrich.footHeight, ostrich.footWidth);
	modelMatrix = mstack.pop();

	//右腿
	mstack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.25f * ostrich.bodyLength * body->getWidth(), 0.0f, 0.35 * ostrich.bodyWidth * body->getLength()- 0.5 * ostrich.legWidth * (rightLeg->getLength())));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.RightLeg]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(rightLeg, modelMatrix, 0.0f, -0.5 * ostrich.legHeight * rightLeg->getHeight(), 0.0f, ostrich.legLength, ostrich.legHeight, ostrich.legWidth);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -ostrich.legHeight * rightLowerLeg->getHeight(), 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.RightLowerLeg]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(rightLowerLeg, modelMatrix, 0.0f, -0.5f * ostrich.lowerLegHeight * rightLowerLeg->getHeight(), 0.0f, ostrich.lowerLegLength, ostrich.lowerLegHeight, ostrich.lowerLegWidth);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5 * ostrich.lowerLegLength * leftLowerLeg->getWidth(), -ostrich.lowerLegHeight * leftLowerLeg->getHeight(), 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.RightFoot]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(rightFoot, modelMatrix, -0.5f * ostrich.footLength * rightFoot->getWidth(), -0.5 * ostrich.footHeight * rightFoot->getHeight(), 0.0f, ostrich.footLength, ostrich.footHeight, ostrich.footWidth);
	modelMatrix = mstack.pop();
 
	// 左翅膀
	mstack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f * ostrich.wingLength * leftWing->getWidth(), ostrich.bodyHeight * 0.5 * body->getHeight(), 0.5 * ostrich.bodyWidth * body->getLength() + 0.5f * ostrich.wingWidth * leftWing->getLength()));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.RightWing]), glm::vec3(0.0, 1.0, 0.0));
	MeshTranslation(leftWing, modelMatrix, 0.5f * ostrich.wingLength * leftWing->getWidth(), 0.0f, 0.5f * ostrich.wingWidth * leftWing->getLength(), ostrich.wingLength, ostrich.wingHeight, ostrich.wingWidth);
	modelMatrix = mstack.pop();

	//右翅膀
	mstack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f * ostrich.wingLength * leftWing->getWidth(), ostrich.bodyHeight * 0.5 * body->getHeight(), -0.5 * ostrich.bodyWidth * body->getLength() - 0.5f * ostrich.wingWidth * leftWing->getLength()));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.LeftWing]), glm::vec3(0.0, 1.0, 0.0));
	MeshTranslation(rightWing, modelMatrix, 0.5f * ostrich.wingLength * leftWing->getWidth(), 0.0f, -0.5f * ostrich.wingWidth * leftWing->getLength(), ostrich.wingLength, ostrich.wingHeight, ostrich.wingWidth);
	modelMatrix = mstack.pop();

	//neck1
	mstack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(ostrich.bodyLength * -0.8 * body->getLength(), ostrich.bodyHeight * body->getHeight(), 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.Neck1]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(neck1, modelMatrix, 0.0f, 0.0f, 0.0f, ostrich.neck1Length, ostrich.neck1Height, ostrich.neck1Width);

	//neck2
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, ostrich.neck1Height * 0.5 * neck1->getHeight(), 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.Neck2]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(neck2, modelMatrix, 0.0f, ostrich.neck2Height * 0.5 * neck2->getHeight(), 0.0f, ostrich.neck2Length, ostrich.neck2Height, ostrich.neck2Width);

	//neck3
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, ostrich.neck2Height * 1.0 * neck2->getHeight(), 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.Neck3]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(neck3, modelMatrix, 0.0f, ostrich.neck3Height * 0.5 * neck3->getHeight(), 0.0f,ostrich.neck3Length, ostrich.neck3Height, ostrich.neck3Width);

	//head/
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, ostrich.neck3Height * 23.0 / 25.0 * neck3->getHeight(), 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.Head]), glm::vec3(0.0, 0.0, 1.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(ostrich.theta[ostrich.Head2]), glm::vec3(1.0, 0.0, 0.0));
	MeshTranslation(hd, modelMatrix, -0.3f * ostrich.headLength * hd->getWidth(), ostrich.headHeight * 0.5 * hd->getHeight(), 0.0f, ostrich.headLength, ostrich.headHeight, ostrich.headWidth);
	modelMatrix = mstack.pop();
}

float carX = -0.111f;
float carY = -0.78f;
float carZ = -3.466f;
void getCarModelMatrix() {
	// 物体的变换矩阵
	glm::mat4 modelMatrix = glm::mat4(1.0);

	// 保持变换矩阵的栈
	MatrixStack mstack;

	// 车身
	modelMatrix = glm::translate(modelMatrix, glm::vec3(carX, carY, carZ));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(glm::vec3(90.0, 0.0f, 0.0f).x), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(car.theta[car.CarBody]), glm::vec3(0.0, 1.0, 0.0));
	MeshTranslation(carBody, modelMatrix, 0.0f, 0.0f, 0.0f, car.carBodyLength, car.carBodyHeight, car.carBodyWidth);
	mstack.push(modelMatrix);

	//左前轮
	modelMatrix = glm::translate(modelMatrix, glm::vec3(car.carBodyLength * carBody->getWidth() * -0.2f, car.carBodyHeight * carBody->getHeight() * -0.5f, car.carBodyWidth * carBody->getLength() * 0.5f + carFLWheel->getLength() * car.wheelWidth * 0.5f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(glm::vec3(0.0, 0.0f, 0.0f).x), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(car.theta[car.WheelY]), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(car.theta[car.WheelZ]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(carFLWheel, modelMatrix, 0.0f, 0.0f, 0.0f, car.wheelLength, car.wheelHeight, car.wheelWidth);
	modelMatrix =  mstack.pop();

	//右前轮
	mstack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(car.carBodyLength * carBody->getWidth() * -0.2f, car.carBodyHeight * carBody->getHeight() * -0.5f, car.carBodyWidth * carBody->getLength() * -0.5f + carFLWheel->getLength() * car.wheelWidth * -0.5f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(glm::vec3(0.0, 0.0f, 0.0f).x), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(car.theta[car.WheelY]), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(car.theta[car.WheelZ]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(carFRWheel, modelMatrix, 0.0f, 0.0f, 0.0f, car.wheelLength, car.wheelHeight, car.wheelWidth);
	modelMatrix = mstack.pop();

	//左后轮
	mstack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(car.carBodyLength * carBody->getWidth() * +0.25f, car.carBodyHeight * carBody->getHeight() * -0.5f, car.carBodyWidth * carBody->getLength() * +0.5f + carFLWheel->getLength() * car.wheelWidth * 0.5f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(glm::vec3(0.0, 0.0f, 0.0f).x), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(car.theta[car.WheelZ]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(carBLWheel, modelMatrix, 0.0f, 0.0f, 0.0f, car.wheelLength, car.wheelHeight, car.wheelWidth);
	modelMatrix = mstack.pop();

	//右后轮
	mstack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(car.carBodyLength * carBody->getWidth() * +0.25f, car.carBodyHeight * carBody->getHeight() * -0.5f, car.carBodyWidth * carBody->getLength() * -0.5f + carFLWheel->getLength() * car.wheelWidth * -0.5f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(glm::vec3(0.0, 0.0f, 0.0f).x), glm::vec3(0.0, 1.0, 0.0));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(car.theta[car.WheelZ]), glm::vec3(0.0, 0.0, 1.0));
	MeshTranslation(carBRWheel, modelMatrix, 0.0f, 0.0f, 0.0f, car.wheelLength, car.wheelHeight, car.wheelWidth);
	modelMatrix = mstack.pop();

	//左灯
	mstack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(car.carBodyLength * carBody->getWidth() * +0.5f, car.carBodyHeight * carBody->getHeight() * -0.34f, car.carBodyWidth * carBody->getLength() * +0.4f));
	MeshTranslation(carLeftLight, modelMatrix, 0.0f, 0.0f, 0.0f, car.lightLength, car.lightHeight, car.lightWidth);
	modelMatrix = mstack.pop();

	//左灯
	mstack.push(modelMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(car.carBodyLength * carBody->getWidth() * +0.5f, car.carBodyHeight * carBody->getHeight() * -0.34f, car.carBodyWidth * carBody->getLength() * -0.4f));
	MeshTranslation(carRightLight, modelMatrix, 0.0f, 0.0f, 0.0f, car.lightLength, car.lightHeight, car.lightWidth);
	modelMatrix = mstack.pop();
}

unsigned int skyboxIndices[] = {
	1,2,6,//right
	6,5,1,
	0,4,7,//left
	7,3,0,
	4,5,6,//top
	6,7,4,
	0,3,2,//bottom
	2,1,0,
	0,1,5,//back
	5,4,0,
	3,7,6,//front
	6,2,3

};

std::string faces[6] = {
	"./assets/skybox/tron_rtt.jpg",// 右边的图需要180读旋转
	"./assets/skybox/tron_lf.png",
	"./assets/skybox/tron_up.png",
	"./assets/skybox/tron_dn.png",
	"./assets/skybox/tron_bk.png",
	"./assets/skybox/tron_ft.png"
};

GLuint program;
GLuint skyboxProgram;
unsigned int vao, vbo, ebo;
unsigned int skyboxTexture;

void bindSkyboxData() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenTextures(1, &skyboxTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	for (unsigned int i = 0; i < 6; i++) {
		int width, height, channels;
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
		if (data) {
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,	
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else {
			std::cout << "Failed to load texture: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void bindObjectAndData(TriMesh* mesh, openGLObject& object,GLuint program) {

	// 创建顶点数组对象
	glGenVertexArrays(1, &object.vao);
	glBindVertexArray(object.vao);

	// 创建并初始化顶点缓存对象
	glGenBuffers(1, &object.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, object.vbo);
	glBufferData(GL_ARRAY_BUFFER,
		mesh->getPoints().size() * sizeof(glm::vec3) + mesh->getColors().size() * sizeof(glm::vec3),
		NULL,
		GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->getPoints().size() * sizeof(glm::vec3), &mesh->getPoints()[0]);
	glBufferSubData(GL_ARRAY_BUFFER, mesh->getPoints().size() * sizeof(glm::vec3), mesh->getColors().size() * sizeof(glm::vec3), &mesh->getColors()[0]);

	object.program = program;

	// 从顶点着色器中初始化顶点的位置
	object.pLocation = glGetAttribLocation(object.program, "vPosition");
	glEnableVertexAttribArray(object.pLocation);
	glVertexAttribPointer(object.pLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	// 从顶点着色器中初始化顶点的颜色
	object.cLocation = glGetAttribLocation(object.program, "vColor");
	glEnableVertexAttribArray(object.cLocation);
	glVertexAttribPointer(object.cLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(mesh->getPoints().size() * sizeof(glm::vec3)));

	// 获得矩阵位置
	object.modelLocation = glGetUniformLocation(object.program, "model");
	object.viewLocation = glGetUniformLocation(object.program, "view");
	object.projectionLocation = glGetUniformLocation(object.program, "projection");

	// 获得阴影标识的位置
	object.shadowLocation = glGetUniformLocation(object.program, "isShadow");
}

bool light_firshClick = false;
void mouse_input(GLFWwindow* window) {
	//按住alt键
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {
			//按住鼠标右键
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
				//隐藏光标
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				//判断是否时初次点击，初次点击则将鼠标移至屏幕中央
				if (light_firshClick) {
					glfwSetCursorPos(window, (WIDTH / 2), (HEIGHT / 2));
					light_firshClick = false;
				}
				//计算光标移动的偏移量
				double posx;
				double posy;
				glfwGetCursorPos(window, &posx, &posy);
				float offsetx = 10.0f * (float)(posy - (HEIGHT / 2)) / HEIGHT;
				float offsety = 10.0f * (float)(posx - (HEIGHT / 2)) / HEIGHT;
				//移动灯光
				glm::vec3 pos = light->getTranslation();
				pos.x -= offsety;
				pos.y -= offsetx;
				
				if (pos.y <= 2.65) pos.y = 2.65;//限制灯光在一定高度
				if (pos.y >= 25) pos.y = 2.65;//当高度达到阈值时，闪现至最低位置
				//墙体具有一定高度，当在墙体高度范围内时，边界具有穿越特性
				if (pos.y > 4.1267) {
					//超出边界时从另一端出来
					if (pos.x > 4.276672f) pos.x = 4.276672f;
					else if (pos.x < -4.276672f) pos.x = -4.276672f;
				}
				else
				{
					//超出边界时不更新灯光位置
					if (pos.x < -4.276672f) pos.x = 4.276670f;
					else if(pos.x >= 4.276672f) pos.x = -4.276670f;

					if (pos.z <= -4.192f) pos.z = 4.190f;
					else if (pos.z >= 4.192f) pos.z = -4.190f;
				}

				//更新灯光位置
				light->setTranslation(pos);
				//刷新灯光模型位置
				lighMesh->setTranslation(light->getTranslation());

				glfwSetCursorPos(window, (WIDTH / 2), (HEIGHT / 2));
			}
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				light_firshClick = true;
			}
		}



	if (glfwGetKey(window,GLFW_KEY_SPACE)==GLFW_PRESS)
	{
		light->setTranslation(glm::vec3(0.0, 6.0, 2.0));
		light->setAmbient(glm::vec4(1.1, 1.1, 1.1, 1.0)); // 环境光
		light->setDiffuse(glm::vec4(1.1, 1.1, 1.1, 1.0)); // 漫反射
		light->setSpecular(glm::vec4(1.1, 1.1, 1.1, 1.0)); // 镜面反射
		light->setAttenuation(1, 0.045, 0.0075);
		lighMesh->setTranslation(glm::vec3(0.0, 4.15, 2.0));
		lighMesh->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 环境光
		lighMesh->setDiffuse(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 漫反射
		lighMesh->setSpecular(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 镜面反射
	}
	}
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


bool die = false;
bool over = false;
bool start = false;
bool waiting = false;
bool overRotate = false;
void init()
{
	painter = new MeshPainter();
	painter2 = new MeshPainter();

	gameTime = 18.0;
	gapTime = gameTime / 3;
	
	camera->eye = glm::vec4(0.094673f, -10.203f, -0.0843f, 1.0f);
	camera->at = glm::vec4(-0.0016f, 0.9979f, 0.061f,1.0f);
	camera->birdLoction = glm::vec4(transX, transY + 0.531, transZ, 1.0f);
	camera->carLoction2 = glm::vec3(carX - 1 * sin(glm::radians(car.theta[0])), carY + 0.531, carZ - 1 * cos(glm::radians(car.theta[0])));

	std::string vshader, fshader, skybox_vs, skybox_fs;
	// 读取着色器并使用
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";
	skybox_vs = "shaders/skybox_vs.glsl";
	skybox_fs = "shaders/skybox_fs.glsl";
	program = InitShader(vshader.c_str(), fshader.c_str());
	skyboxProgram = InitShader(skybox_vs.c_str(), skybox_fs.c_str());

	camera->firstClick = true;

	// 设置光源位置
	light->setTranslation(glm::vec3(0.0, 10.3, 2.0));
	light->setAmbient(glm::vec4(1.2, 1.2, 1.2, 1.0)); // 环境光
	light->setDiffuse(glm::vec4(1.2, 1.2, 1.2, 1.0)); // 漫反射
	light->setSpecular(glm::vec4(1.2, 1.2, 1.2, 1.0)); // 镜面反射
	light->setAttenuation(1, 0.045, 0.0075); // 衰减系数
	light->setShininess(1);

	bool havaShadow = true;
	//box
	TriMesh* box = new TriMesh();
	box->robot = false;
	box->setNormalize(true);
	box->readObj("./assets/squid_game/box.obj");
	box->setTranslation(glm::vec3(0.0, 1.469, 0.0));
	box->setRotation(glm::vec3(0.0, 0.0, 0.0));
	box->setScale(glm::vec3(13.0, 13.0, 13.0));
	box->readMtl("./assets/squid_game/box.mtl");
	box->setAmbient(glm::vec4(0.6, 0.6, 0.6,1.0));
	painter->addMesh(box, "box", "./assets/squid_game/box.png", false, vshader, fshader);
	
	//helloworld
	TriMesh* helloworld = new TriMesh();
	helloworld->setNormalize(true);
	helloworld->readObj("./assets/squid_game/helloworld.obj");
	helloworld->setTranslation(glm::vec3(-0.0, -1.159, -0.0));
	helloworld->setRotation(glm::vec3(0.0, 180.0, 0.0));
	helloworld->setScale(glm::vec3(11.85, 11.85, 11.85));
	helloworld->readMtl("./assets/squid_game/helloworld.mtl");
	helloworld->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0));
	painter->addMesh(helloworld, "helloworld", "./assets/squid_game/helloworld.png", false, vshader, fshader);
	
	//sse
	TriMesh* sse = new TriMesh();
	sse->setNormalize(true);
	sse->readObj("./assets/squid_game/sse.obj");
	sse->setTranslation(glm::vec3(-0.0, -1.209, 2.44));
	sse->setRotation(glm::vec3(0.0, 0.0, 0.0));
	sse->setScale(glm::vec3(3.7, 3.7, 3.7));
	sse->readMtl("./assets/squid_game/sse.mtl");
	sse->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0));
	painter->addMesh(sse, "sse", "./assets/squid_game/sse.png", false, vshader, fshader);

	//tree
	TriMesh* tree = new TriMesh();
	tree->setNormalize(true);
	tree->readObj("./assets/squid_game/dead_tree.obj");
	tree->setTranslation(glm::vec3(0.0, 0.17, 3.11));
	tree->setRotation(glm::vec3(0.0, 95.0, 0.0));
	tree->setScale(glm::vec3(4.1, 4.1, 4.1));
	tree->readMtl("./assets/squid_game/dead_tree.mtl");
	tree->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter->addMesh(tree, "tree", "./assets/squid_game/dead_tree.png", havaShadow, vshader, fshader);
	meshList.push_back(tree);
	

	//wawa_black
	wawa_black = new TriMesh();
	wawa_black->setNormalize(true);
	wawa_black->readObj("./assets/squid_game/wawa_black.obj");
	wawa_black->setTranslation(glm::vec3(0.0, -0.33, 2.16));
	wawa_black->setRotation(glm::vec3(0.0, 0.0, 0.0));
	wawa_black->setScale(glm::vec3(1.7, 1.7, 1.7));
	wawa_black->readMtl("./assets/squid_game/wawa_black.mtl");
	wawa_black->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter->bindObjectAndData(wawa_black, wawa_black->object, "./assets/squid_game/wawa_black.png", vshader, fshader);
	meshList.push_back(wawa_black);

	currentWawa = wawa_black;
	currentWawa->object.haveShadow = true;
	//wawa_red
	wawa_red = new TriMesh();
	wawa_red->setNormalize(true);
	wawa_red->readObj("./assets/squid_game/wawa_red.obj");
	wawa_red->setTranslation(glm::vec3(0.0, -0.33, 2.16));
	wawa_red->setRotation(glm::vec3(0.0, 180.0, 0.0));
	wawa_red->setScale(glm::vec3(0, 0, 0));
	wawa_red->readMtl("./assets/squid_game/wawa_black.mtl");
	wawa_red->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter->bindObjectAndData(wawa_red, wawa_red->object, "./assets/squid_game/wawa_black.png", vshader, fshader);
	meshList.push_back(wawa_red);

	//wawa_joker
	wawa_joker = new TriMesh();
	wawa_joker->setNormalize(true);
	wawa_joker->readObj("./assets/squid_game/wawa_joker.obj");
	wawa_joker->setTranslation(glm::vec3(0.0, -0.33, 2.16));
	wawa_joker->setRotation(glm::vec3(0.0, 180.0, 0.0));
	wawa_joker->setScale(glm::vec3(0.0f, 0.0f, 0.0f));
	wawa_joker->readMtl("./assets/squid_game/wawa_joker.mtl");
	wawa_joker->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter->bindObjectAndData(wawa_joker, wawa_joker->object, "./assets/squid_game/wawa_joker.png", vshader, fshader);
	meshList.push_back(wawa_joker);

	//wawa_joker_girl
	wawa_joker_girl = new TriMesh();
	wawa_joker_girl->setNormalize(true);
	wawa_joker_girl->readObj("./assets/squid_game/wawa_joker_girl.obj");
	wawa_joker_girl->setTranslation(glm::vec3(0.0, -0.33, 2.16));
	wawa_joker_girl->setRotation(glm::vec3(0.0, 180.0, 0.0));
	wawa_joker_girl->setScale(glm::vec3(0.0f, 0.0f, 0.0f));
	wawa_joker_girl->readMtl("./assets/squid_game/wawa_joker_girl.mtl");
	wawa_joker_girl->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter->bindObjectAndData(wawa_joker_girl, wawa_joker_girl->object, "./assets/squid_game/wawa_joker_girl.png", vshader, fshader);
	meshList.push_back(wawa_joker_girl);
	
	//npc_right
	TriMesh* npc_right = new TriMesh();
	npc_right->setNormalize(true);
	npc_right->readObj("./assets/squid_game/npc_yuan.obj");
	npc_right->setTranslation(glm::vec3(-1.65f, -0.61f, 2.85f));
	npc_right->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	npc_right->setScale(glm::vec3(1.25f, 1.25f, 1.25f));
	npc_right->readMtl("./assets/squid_game/npc_yuan.mtl");
	painter->addMesh(npc_right, "npc_right", "./assets/squid_game/npc_yuan.png", havaShadow, vshader, fshader);
	meshList.push_back(npc_right);

	//npc_left
	TriMesh* npc_square = new TriMesh();
	npc_square->setNormalize(true);
	npc_square->readObj("./assets/squid_game/npc_square.obj");
	npc_square->setTranslation(glm::vec3(1.65f, -0.61f, 2.85f));
	npc_square->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	npc_square->setScale(glm::vec3(1.25f, 1.25f, 1.25f));
	npc_square->readMtl("./assets/squid_game/npc_szu.mtl");
	npc_square->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter->addMesh(npc_square, "npc_square", "./assets/squid_game/npc_square.png", havaShadow,vshader, fshader);
	meshList.push_back(npc_square);
	

	//coffin
	TriMesh* coffin = new TriMesh();
	coffin->setNormalize(true);
	coffin->readObj("./assets/squid_game/coffin.obj");
	//coffin->setTranslation(glm::vec3(-0.25, -0.88, -0.65));
	coffin->setRotation(glm::vec3(0.0, -305.0, 0.0));
	//coffin->setScale(glm::vec3(2.35, 2.35, 2.35));
	coffin->setScale(glm::vec3(0.0f, 0.0f, 0.0f));
	coffin->readMtl("./assets/squid_game/coffin.mtl");
	coffin->setAmbient(glm::vec4(0.85, 0.85, 0.85, 1.0));
	painter->addMesh(coffin, "coffin", "./assets/squid_game/coffin.png", false, vshader, fshader);

	//light
	lighMesh = new TriMesh();
	lighMesh->setNormalize(true);
	lighMesh->readObj("./assets/squid_game/lamp.obj");
	lighMesh->setTranslation(light->getTranslation());
	lighMesh->setRotation(glm::vec3(0.0, -180.0, 0.0));
	lighMesh->setScale(glm::vec3(0.5, 0.5, 0.5));
	glm::vec4 tmp = glm::vec4(2.0, 2.0, 2.0, 1.0);
	lighMesh->setAmbient(tmp*light->getAmbient());
	lighMesh->setDiffuse(tmp* light->getDiffuse());
	lighMesh->setSpecular(tmp* light->getDiffuse());
	painter->addMesh(lighMesh, "lighMesh", "./assets/squid_game/lamp.png", false, vshader, fshader);

	//brand3
	brand3 = new TriMesh();
	brand3->setNormalize(true);
	brand3->readObj("./assets/squid_game/brand3.obj");
	brand3->setTranslation(glm::vec3(-0.25, 2.1, 4.15));
	brand3->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	brand3->setScale(glm::vec3(2.6, 2.6, 2.6));
	brand3->readMtl("./assets/squid_game/brand3.mtl");
	painter->bindObjectAndData(brand3, brand3->object, "./assets/squid_game/brand3.png", vshader, fshader);
	currentBrand = brand3;
	currentBrand->object.haveShadow = false;
	//brand2
	brand2 = new TriMesh();
	brand2->setNormalize(true);
	brand2->readObj("./assets/squid_game/brand2.obj");
	brand2->setTranslation(glm::vec3(-0.25, 2.1, 4.15));
	brand2->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	brand2->setScale(glm::vec3(0.0, 0.0, 0.0));
	brand2->readMtl("./assets/squid_game/brand2.mtl");
	painter->bindObjectAndData(brand2, brand2->object, "./assets/squid_game/brand2.png", vshader, fshader);
	//brand1
	brand1 = new TriMesh();
	brand1->setNormalize(true);
	brand1->readObj("./assets/squid_game/brand1.obj");
	brand1->setTranslation(glm::vec3(-0.25, 2.1, 4.15));
	brand1->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	brand1->setScale(glm::vec3(0.0, 0.0, 0.0));
	brand1->readMtl("./assets/squid_game/brand1.mtl");
	painter->bindObjectAndData(brand1, brand1->object, "./assets/squid_game/brand1.png", vshader, fshader);
	//brand0
	brand0 = new TriMesh();
	brand0->setNormalize(true);
	brand0->readObj("./assets/squid_game/brand0.obj");
	brand0->setTranslation(glm::vec3(-0.25, 2.1, 4.15));
	brand0->setRotation(glm::vec3(-90.0, 0.0, 0.0));
	brand0->setScale(glm::vec3(0.0, 0.0, 0.0));
	brand0->readMtl("./assets/squid_game/brand0.mtl");
	painter->bindObjectAndData(brand0, brand0->object, "./assets/squid_game/brand0.png", vshader, fshader);

	mesh_coffin = coffin;
	mesh_npc1 = npc_square;
	mesh_npc2 = npc_right;
	
	/*--------------------------------------------------------------------==*/
	
	bool isrobot = true;// 是否是层级建模模型
	

	body->robot = isrobot;
	body->readObj("./assets/ostrich/body.obj");
	body->readMtl("./assets/ostrich/body.mtl");
	body->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(body, "body", "./assets/ostrich/body.png", ostrich.shadow, vshader, fshader);
	getSize(body);



	leftLeg->robot = isrobot;
	leftLeg->readObj("./assets/ostrich/leg.obj");
	leftLeg->readMtl("./assets/ostrich/leg.mtl");
	leftLeg->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(leftLeg, "leftLeg", "./assets/ostrich/leg.png", ostrich.shadow, vshader, fshader);
	getSize(leftLeg);

	rightLeg->robot = isrobot;
	rightLeg->readObj("./assets/ostrich/leg.obj");
	rightLeg->readMtl("./assets/ostrich/leg.mtl");
	rightLeg->setScale(glm::vec3(1, 1, 1));
	rightLeg->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(rightLeg, "rightLeg", "./assets/ostrich/leg.png", ostrich.shadow, vshader, fshader);
	getSize(rightLeg);

	leftLowerLeg->robot = isrobot;
	leftLowerLeg->readObj("./assets/ostrich/foot4.obj");
	leftLowerLeg->readMtl("./assets/ostrich/foot4.mtl");
	leftLowerLeg->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(leftLowerLeg, "leftLowerLeg", "./assets/ostrich/foot4.png", ostrich.shadow, vshader, fshader);
	getSize(leftLowerLeg);

	rightLowerLeg->robot = isrobot;
	rightLowerLeg->readObj("./assets/ostrich/foot4.obj");
	rightLowerLeg->readMtl("./assets/ostrich/foot4.mtl");
	rightLowerLeg->setScale(glm::vec3(1, 1, 1));
	rightLowerLeg->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(rightLowerLeg, "rightLowerLeg", "./assets/ostrich/foot4.png", ostrich.shadow, vshader, fshader);
	getSize(rightLowerLeg);

	rightLowerLeg->robot = isrobot;
	rightLowerLeg->readObj("./assets/ostrich/foot4.obj");
	rightLowerLeg->readMtl("./assets/ostrich/foot4.mtl");
	rightLowerLeg->setScale(glm::vec3(1, 1, 1));
	rightLowerLeg->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(rightLowerLeg, "rightLowerLeg", "./assets/ostrich/foot4.png", ostrich.shadow, vshader, fshader);
	getSize(rightLowerLeg);

	leftFoot->robot = isrobot;
	leftFoot->readObj("./assets/ostrich/foot2.obj");
	leftFoot->readMtl("./assets/ostrich/foot2.mtl");
	leftFoot->setScale(glm::vec3(1, 1, 1));
	leftFoot->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(leftFoot, "rightLowerLeg", "./assets/ostrich/foot2.png", ostrich.shadow, vshader, fshader);
	getSize(leftFoot);

	rightFoot->robot = isrobot;
	rightFoot->readObj("./assets/ostrich/foot2.obj");
	rightFoot->readMtl("./assets/ostrich/foot2.mtl");
	rightFoot->setScale(glm::vec3(1, 1, 1));
	rightFoot->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(rightFoot, "rightFoot", "./assets/ostrich/foot2.png", ostrich.shadow, vshader, fshader);
	getSize(rightFoot);

	leftWing->robot = isrobot;
	leftWing->readObj("./assets/ostrich/wingLeft.obj");
	leftWing->readMtl("./assets/ostrich/wing.mtl");
	leftWing->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(leftWing, "leftWing", "./assets/ostrich/wingLeft.png", ostrich.shadow, vshader, fshader);
	getSize(leftWing);

	rightWing->robot = isrobot;
	rightWing->readObj("./assets/ostrich/wingRight.obj");
	rightWing->readMtl("./assets/ostrich/wing.mtl");
	rightWing->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(rightWing, "rightWing", "./assets/ostrich/wingRight.png", ostrich.shadow, vshader, fshader);
	getSize(leftLeg);

	neck1->robot = isrobot;
	neck1->readObj("./assets/ostrich/neckBottom.obj");
	neck1->readMtl("./assets/ostrich/neckBottom.mtl");
	neck1->setScale(glm::vec3(1, 1, 1));
	neck1->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(neck1, "neck1", "./assets/ostrich/neckBottom.png", ostrich.shadow, vshader, fshader);
	getSize(neck1);

	neck2->robot = isrobot;
	neck2->readObj("./assets/ostrich/neckLower.obj");
	neck2->readMtl("./assets/ostrich/neckLower.mtl");
	neck2->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(neck2, "neck2", "./assets/ostrich/neckLower.png", ostrich.shadow, vshader, fshader);
	getSize(neck2);

	neck3->robot = isrobot;
	neck3->readObj("./assets/ostrich/neckUpper.obj");
	neck3->readMtl("./assets/ostrich/neckCenter.mtl");
	neck3->setScale(glm::vec3(1, 1, 1));
	neck3->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(neck3, "neck3", "./assets/ostrich/neckUpper.png", ostrich.shadow, vshader, fshader);
	getSize(neck3);

	hd->robot = isrobot;
	hd->readObj("./assets/ostrich/head.obj");
	hd->readMtl("./assets/ostrich/head.mtl");
	hd->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter2->addMesh(hd, "head", "./assets/ostrich/head.png", ostrich.shadow, vshader, fshader);
	getSize(hd);
	
	/*--------------------------------------------------------------------=*/
	carBody->robot = isrobot;
	carBody->readObj("./assets/squid_game/carbody.obj");
	carBody->readMtl("./assets/squid_game/carbody.mtl");
	carBody->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter3->addMesh(carBody, "carBody", "./assets/squid_game/carbody.png", car.shadow, vshader, fshader);
	getSize(carBody);

	carFLWheel->robot = isrobot;
	carFLWheel->readObj("./assets/squid_game/wheel3.obj");
	carFLWheel->readMtl("./assets/squid_game/wheel.mtl");
	carFLWheel->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter3->addMesh(carFLWheel, "carFLWheel", "./assets/squid_game/wheel3.png", car.shadow, vshader, fshader);
	getSize(carFLWheel);

	carFRWheel->robot = isrobot;
	carFRWheel->readObj("./assets/squid_game/wheel2.obj");
	carFRWheel->readMtl("./assets/squid_game/wheel.mtl");
	carFRWheel->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter3->addMesh(carFRWheel, "carFRWheel", "./assets/squid_game/wheel2.png", car.shadow, vshader, fshader);
	getSize(carFRWheel);

	carBLWheel->robot = isrobot;
	carBLWheel->readObj("./assets/squid_game/wheel3.obj");
	carBLWheel->readMtl("./assets/squid_game/wheel.mtl");
	carBLWheel->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter3->addMesh(carBLWheel, "carBLWheel", "./assets/squid_game/wheel3.png", car.shadow, vshader, fshader);
	getSize(carBLWheel);

	carBRWheel->robot = isrobot;
	carBRWheel->readObj("./assets/squid_game/wheel2.obj");
	carBRWheel->readMtl("./assets/squid_game/wheel.mtl");
	carBRWheel->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter3->addMesh(carBRWheel, "carBRWheel", "./assets/squid_game/wheel2.png", car.shadow, vshader, fshader);
	getSize(carBRWheel);

	carLeftLight->robot = isrobot;
	carLeftLight->readObj("./assets/squid_game/carLight.obj");
	carLeftLight->readMtl("./assets/squid_game/carLight.mtl");
	carLeftLight->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter3->addMesh(carLeftLight, "carLeftLight", "./assets/squid_game/carLight.png", car.shadow, vshader, fshader);
	getSize(carLeftLight);

	carRightLight->robot = isrobot;
	carRightLight->readObj("./assets/squid_game/carLight.obj");
	carRightLight->readMtl("./assets/squid_game/carLight.mtl");
	carRightLight->setAmbient(glm::vec4(0.6, 0.6, 0.6, 1.0));
	painter3->addMesh(carRightLight, "carRightLight", "./assets/squid_game/carLight.png", car.shadow, vshader, fshader);
	getSize(carRightLight);
	/*--------------------------------------------------------------------*/

	for (int i = 0; i < meshList.size(); i++) {
		getSize(meshList[i]);
	}
	bindSkyboxData();
	glClearColor(1.0, 1.0, 0.2, 1.0);
}


void display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	painter->drawMeshes(light, camera);
	glUseProgram(program);
	painter->drawMesh(wawa_black, wawa_black->object, light, camera);
	painter->drawMesh(wawa_red, wawa_red->object, light, camera);
	painter->drawMesh(wawa_joker, wawa_joker->object, light, camera);
	painter->drawMesh(wawa_joker_girl, wawa_joker_girl->object, light, camera);
	painter->drawMesh(brand3, brand3->object, light, camera);
	painter->drawMesh(brand2, brand2->object, light, camera);
	painter->drawMesh(brand1, brand1->object, light, camera);
	painter->drawMesh(brand0, brand0->object, light, camera);
	if (!die) {
		if (modelIsOstrich) {
			getRobotModelMatrix();
			painter2->drawMeshes(light, camera);
		}
		else {
			getCarModelMatrix();
			painter3->drawMeshes(light, camera);
		}
	}
	glBindVertexArray(0);

	glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GLFW_FALSE);
	glUseProgram(skyboxProgram);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0);
	view = glm::mat4(glm::mat3(camera->getViewMatrix()));
	projection = camera->getProjectionMatrix(false);
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glDepthMask(GL_TRUE);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);

}

bool checkCollision(float newx,float newz,float angle) {
	//获取当前控制模型的尺寸
	float currentLenght, currentWidth;
	currentLenght = carBody->getLength();
	currentWidth = carBody->getWidth();
	float w1, w2, l1, l2;
	//根据角度计算矩形在X,Z轴方向上的范围
	w1 = abs(currentLenght * cos(glm::radians(angle)));//Z轴
	w2 = abs(currentWidth * sin(glm::radians(angle)));
	l1 = abs(currentWidth * cos(glm::radians(angle)));//X轴
	l2 = abs(currentLenght * sin(glm::radians(angle)));
	//选取在X、Z轴分量较大的作为碰撞范围
	float length = w1>w2?w1:w2;
	float width = l1>l2?l1:l2;
	float x = newx;
	float z = newz;
	//判断是否碰撞
	bool flag = false;
	//获取各个物体的尺寸
	float wmesh;//宽
	float lmesh;//长
	float xmesh;//x坐标
	float zmesh;//z坐标
	bool c1, c2;//x坐标与z坐标分别是否发生碰撞
	for (int i = 0; i < meshList.size(); i++) {//遍历地图上的物体
		if (i == 0) {//由于树的模型复杂，且枝干范围较大，因此手动调试树干尺寸的比例系数
			wmesh = meshList[i]->getWidth() * 0.2;
			lmesh = meshList[i]->getLength() * 0.2;
			xmesh = meshList[i]->getTranslation().x - 0.16;
			zmesh = meshList[i]->getTranslation().z - 0.22;
		}
		else {//0.4为减小矩形范围
			wmesh = meshList[i]->getWidth() * 0.4f * meshList[i]->getScale().x;
			lmesh = meshList[i]->getLength() * 0.4f * meshList[i]->getScale().x;
			xmesh = meshList[i]->getTranslation().x;
			zmesh = meshList[i]->getTranslation().z;
		}
		c1 = c2 = false;//初始化c1,c2
		c1 = (xmesh + lmesh / 2) > (x - length / 2) && (x + length / 2) > (xmesh - lmesh / 2);
		if (c1) {//如果X轴方向碰撞，再检测Z轴方向，否则不检测
			c2 = (zmesh + wmesh / 2) > (z - width / 2) && (z + width / 2) > (zmesh - wmesh / 2);
			if (c2) {//如果Z轴方向也碰撞，则物体发生碰撞
				flag = true;//结束循环
				break;
			}
		}
	}
	return flag;
}

void printHelp()
{
	std::cout << "=============================================================================================" << std::endl;
	std::cout << "|                                           WINDOW                                          |" << std::endl;
	std::cout << "|===========================================================================================|" << std::endl;
	std::cout << "|ESC:                                   Exit                                                |" << std::endl;
	std::cout << "|H:                                     Print help message                                  |" << std::endl;
	std::cout << "|===========================================================================================|" << std::endl;
	std::cout << "|                                           LIGHT                                           |" << std::endl;
	std::cout << "|===========================================================================================|" << std::endl;
	std::cout << "|ALT+MOUSE_LEFT_BUTTON:                 Move the light source in the XY plane               |" << std::endl;
	std::cout << "|ALT+MOUSE_WHEEL:                       Move the light source in the Z-axis                 |" << std::endl;
	std::cout << "|SHIFT+1/2/3+MOUSE_WHEEL:               Change the light source R/G/B value                 |" << std::endl;
	std::cout << "|SPACE+SHIFT:                           Reset light source parameters                       |" << std::endl;
	std::cout << "|===========================================================================================|" << std::endl;
	std::cout << "|                                          CAMERA                                           |" << std::endl;
	std::cout << "|===========================================================================================|" << std::endl;
	std::cout << "|W/S/D/A:                               Move the camera FRONT/BACK/RIGHT/LEFT               |" << std::endl;
	std::cout << "|Q/E:                                   Move the camera UP/DOWN                             |" << std::endl;
	std::cout << "|MOUSE_LEFT_BUTTON:                     Increase/Decrease the rotate angle                  |" << std::endl;
	std::cout << "|KP_0:                                  Follow the model perspective                        |" << std::endl;
	std::cout << "|KP_1:                                  Free perspective                                    |" << std::endl;
	std::cout << "|KP_2:                                  God's perspective                                   |" << std::endl;
	std::cout << "|KP_3/4/5:                              Camera positioning                                  |" << std::endl;
	std::cout << "|KP_6:                                  Unrequited love perspective                         |" << std::endl;
	std::cout << "|KP_7/8/9:                              Switch the camera's FOV value                       |" << std::endl;
	std::cout << "|SHIFT+MOUSE_WHEEL:                     Change the camera's FOV value                       |" << std::endl;
	std::cout << "|DELETE:                                Limit the camera                                    |" << std::endl;
	std::cout << "|BACKSPACE:                             Unlimit the camera                                  |" << std::endl;
	std::cout << "|TAB:                                   Switch to first/third person(when model for car)    |" << std::endl;
	std::cout << "|SPACE:                                 Reset camera parameters                             |" << std::endl;
	std::cout << "|===========================================================================================|" << std::endl;
	std::cout << "|                                        NPC MODEL                                          |" << std::endl;
	std::cout << "|===========================================================================================|" << std::endl;
	std::cout << "|Z/X/C/V:                               Switch NPC model                                    |" << std::endl;
	std::cout << "|M:                                     Switch NPC model in the game                        |" << std::endl;
	std::cout << "|K/L:                                   Increase/Decrease the rotate angle                  |" << std::endl;
	std::cout << "|===========================================================================================|" << std::endl;
	std::cout << "|                                      PLAYER MODEL                                         |" << std::endl;
	std::cout << "|===========================================================================================|" << std::endl;
	std::cout << "|SHIFT:                                 Speed up(when model for car)                        |" << std::endl;
	std::cout << "|SHIFT+CTRL:                            Speed up PLUS(when model for car)                   |" << std::endl;
	std::cout << "|UP/DOWN/RIGHT/LEFT:                    Control model                                       |" << std::endl;
	std::cout << "|U/I:                                   Switch model Ostrich/Car                            |" << std::endl;
	std::cout << "=============================================================================================" << std::endl << std::endl;
}

void changeWawa(int type) {
	glm::vec3 rotate = currentWawa->getRotation();
	glm::vec3 scale = currentWawa->getScale();
	currentWawa->object.haveShadow = false;
	currentWawa->setScale(glm::vec3(0.0f, 0.0f, 0.0f));
	switch (type)
	{
	case 0: currentWawa = wawa_black; break;
	case 1:currentWawa = wawa_red; break;
	case 2:currentWawa = wawa_joker; break;
	case 3:currentWawa = wawa_joker_girl; break;
	}
	currentWawa->setRotation(rotate);
	currentWawa->setScale(scale);
	currentWawa->object.haveShadow = true;
}

void changeBrand(int type) {
	glm::vec3 scale = currentBrand->getScale();
	currentBrand->object.haveShadow = false;
	currentBrand->setScale(glm::vec3(0.0f, 0.0f, 0.0f));
	switch (type)
	{
	case 0: currentBrand = brand0; break;
	case 1:currentBrand = brand1; break;
	case 2:currentBrand = brand2; break;
	case 3:currentBrand = brand3; break;
	}
	currentBrand->setScale(scale);
	currentBrand->object.haveShadow = false;
}

int brandState = 3;
void updateBrand() {
	 changeBrand(--brandState);
}

bool defaultWawaMesh = true;
//玩偶转身
float wawaRotate = 0.0f;

float startTime = 0.0f;
float keepState1Time = 3.0f;
float keepState2Time = 2.0f;
//-1:面向树
//0:旋转中
//1:面向门
int state = -1;
int nextstate = 1;
/*
游戏开始时，控制娃娃的转身时间，速度
同时，娃娃转过身时，检测玩家是否移动
*/
void wawaRotateControl() {
	if (start  && !over) {//判断游戏状态
		float currentTime = glfwGetTime();//获取当前时间戳
		glm::vec3 angle = currentWawa->getRotation();
		if (currentTime - gameStart >= gapTime) {//查看时间戳间距，判断是否更新红绿灯挂牌
			if (brandState != 1) {//如果更新后还有时间，则直接更新
				updateBrand();
				gameStart = currentTime;
			}
			else if (state == 1) {//如果时间结束，则在木头人转身时，延缓一段时间再更新时间
				if(currentTime - startTime >= (keepState2Time/2)) updateBrand();
			}
		}

		if (state == -1) {
			if (currentTime - startTime >= keepState1Time) {
				state = 0;
			}
		}
		else if (state == 1) {
			if (currentTime - startTime >= keepState2Time) {
				state = 0;
			}
		}
		else {
			angle.y += 7.5f;
			if (angle.y >= 360.0f) angle.y -= 360.0f;
			currentWawa->setRotation(angle);
			if (angle.y == 0.0f) { state = -1; startTime = currentTime; }
			else if (angle.y == 180.0f) {
				state = 1; 
				startTime = currentTime;
			}
		}
		if (modelIsOstrich) {
			if (!over && ((state == 1 && (ostrich.move1state != 0 || ostrich.move3state != 0 || ostrich.move4state != 0 || ostrich.move5state != 0)) || brandState==0)) {
					die = true;//死亡状态
					over = true;//结束状态为true
					changeBrand(0);//熄灭所有灯
					mesh_coffin->setScale(glm::vec3(0.95f, 0.95f, 0.95f));//放大棺材模型
					mesh_coffin->object.haveShadow = true;
					mesh_coffin->setTranslation(glm::vec3(transX, -1.008, transZ));//定位至层级模型位置
					mesh_coffin->setRotation(glm::vec3(0.0f, -222.5f + ostrich.theta[0], 0.0f)) ;//旋转棺材方向使其与层级模型方向一致
					camera->cameraNumber = 1;//相机变为自由模式
					camera->fixed = false;//解除相机旋转限制
			}
		}
		else {
			if (!over && (state==1 && (car.front || car.back || car.left || car.right) || brandState == 0)) {
				die = true;
				over = true;
				changeBrand(0);
				mesh_coffin->setScale(glm::vec3(0.95f, 0.95f, 0.95f));
				mesh_coffin->object.haveShadow = true;
				mesh_coffin->setTranslation(glm::vec3(carX, -1.008, carZ));
				mesh_coffin->setRotation(glm::vec3(0.0f, -222.5f + car.theta[0], 0.0f));
				camera->cameraNumber = 1;
				camera->fixed = false;
			}
		}
			
		if (state == 1) {
			if (defaultWawaMesh) {
				changeWawa(1);
			}
			else {
				changeWawa(2);
			}
		}
		else if (state == -1) {
			if (defaultWawaMesh) {
				changeWawa(0);
			}
			else {
				changeWawa(3);
			}
		}
	}
	if (over && !die && !waiting) {
		changeBrand(3);
		currentWawa->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
		waiting = true;
	}
}

void reset() {
	if (over) {
		//游戏状态重置
		die = false;
		over = false;
		start = false;
		waiting = false;
		overRotate = false;
		//死亡物体重置
		mesh_coffin->setScale(glm::vec3(0.0f, 0.0f, 0.0f));
		mesh_coffin->object.haveShadow = false;
		//鸵鸟位置重置
		transX = -0.111f;
		transZ = -3.466f;
		carX = -0.111f;
		carZ = -3.466f;
		ostrich.loading = 0.0f;
		ostrich.wingDire = 1;
		ostrich.d2_1 = -1;
		ostrich.d2_2 = -1;
		ostrich.d2_3 = 1;
		ostrich.d3_1 = 1;
		ostrich.d3_2 = 1;
		ostrich.d3_3 = -1;
		ostrich.d3_4 = -1;
		ostrich.d4_1 = 1;
		ostrich.d4_2 = 1;
		ostrich.d4_3 = -1;
		ostrich.d4_4 = -1;
		ostrich.d5 = 1;
		ostrich.loading = 0.0f;
		GLfloat resetTheta[14] = {
		0.0f, -40.0f, 0.0f, 0.0f, 0.0f, -20.0f, 15.0f ,50.0f, 55.0f, 55.0f, 270.0f, 270.0f, 35.0f, 35.0f
		};
		for (int i = 0; i < 14; i++) {
			ostrich.theta[i] = resetTheta[i];
		}
		car.theta[car.CarBody] = 0.0f;
		car.theta[car.WheelZ] = 0.0f;
		car.theta[car.WheelY] = 0.0f;
		if (defaultWawaMesh) changeWawa(0);
		else changeWawa(3);
		currentWawa->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		state = -1;
		//npc状态重置
		mesh_npc1->setRotation(glm::vec3(0.0f,0.0f,0.0f));
		mesh_npc2->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		//计时器重置
		gapTime = gameTime / 3;
		changeBrand(3);
		brandState = 3;
	}
}

void cleanData() {
	delete camera;
	camera = NULL;

	delete light;
	light = NULL;

	painter->cleanMeshes();

	delete painter;
	delete painter2;
	painter = NULL;
	painter2 = NULL;

	for (int i = 0; i < meshList.size(); i++) {
		delete meshList[i];
	}
	meshList.clear();
}

void cameraMove9() {
	//相机编号不是0则不改变相机参数
	if (camera->cameraNumber != 0) return ;
	//更具车辆旋转角度，计算相机位置坐标
	glm::vec3 loc = glm::vec3(carX + car.cameraLoc * sin(glm::radians(car.theta[0])), 
		carY + 0.331, carZ + car.cameraLoc * cos(glm::radians(car.theta[0])));
	//防止相机穿过墙体导致视线被遮挡，因此增加一些限制
	loc.x = loc.x > 4.079 ? 4.079 : loc.x;
	loc.x = loc.x < -4.079 ? -4.079 : loc.x;
	loc.z = loc.z > 4.027 ? 4.027 : loc.z;
	loc.z = loc.z < -4.027? -4.027 : loc.z;
	//更新相机位置
	camera->eye = glm::vec4(loc,1.0f);
	//fixed为相机类的参数，表示是否固定相机角度，为true时无法旋转相机
	camera->fixed = true;
}

void key_input(GLFWwindow* window)
{
	float tmp;
	glm::vec4 ambient;
	if (true) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) exit(EXIT_SUCCESS);
		if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) printHelp();

		if (!die) {
			//鸵鸟控制
			if (modelIsOstrich) {
				if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && ostrich.move4state == 0) {//判断是否啄击
					if (ostrich.move1state != 2 && ostrich.move2state != 2 && ostrich.move3state != 2) {//判断是否动作正在恢复中
						ostrich.move1state = 1;
						ostrich.move2state = 1;
						ostrich.move3state = 1;
						float newX, newZ;
						newZ = transZ + (ostrich.walkSpeed * cos(glm::radians(ostrich.theta[0])));
						if (newZ >= 3.525) newZ = 3.525f;
						else if (newZ <= -3.466) newZ = -3.466f;
						newX = transX + (ostrich.walkSpeed * sin(glm::radians(ostrich.theta[0])));
						if (newX >= 3.525) newX = 3.525f;
						else if (newX <= -3.536) newX = -3.536f;
						//无碰撞检测
						transZ = newZ;
						transX = newX;
						camera->birdLoction = glm::vec3(transX, transY + 0.531, transZ);
						if (transZ > -2.87) {
							if (!start && !over) {
								startTime = glfwGetTime();
								gameStart = glfwGetTime();
								currentWawa->setRotation(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
								mesh_npc1->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
								mesh_npc2->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
								start = true;
							}
						}
						if (transZ > 1.8626) {
							over = true;
							start = false;
						}
					}
				}
				else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
					if (ostrich.move1state == 1 && ostrich.move4state == 0 &&
						glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_PRESS &&
						glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_PRESS) ostrich.move1state = 2;
					if (ostrich.move2state == 1) ostrich.move2state = 2;
					if (ostrich.move3state == 1) ostrich.move3state = 2;
				}

				if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && ostrich.move3state == 0) {
					if (ostrich.move1state != 2 && ostrich.move4state != 2) {
						ostrich.move1state = 1;
						ostrich.move4state = 1;
					}
				}
				else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
					if (ostrich.move1state == 1 && ostrich.move3state == 0 &&
						glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_PRESS &&
						glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_PRESS) ostrich.move1state = 2;
					if (ostrich.move4state == 1) ostrich.move4state = 2;
				}

				if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
					float newAngle = ostrich.theta[ostrich.Body] + ostrich.rotateSpeed;
					//无碰撞检测
					ostrich.theta[ostrich.Body] = newAngle;
					if (camera->cameraNumber == 0) {
						glm::vec3 newAt = glm::rotate(glm::vec3(camera->at), glm::radians(ostrich.rotateSpeed), (glm::vec3)camera->up);
						camera->at = glm::vec4(newAt, 1.0);
					}
					if (ostrich.move1state != 2) {
						ostrich.move1state = 1;
					}
				}
				else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
					if (ostrich.move1state == 1 && ostrich.move3state == 0 && ostrich.move4state == 0 &&
						glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_PRESS) ostrich.move1state = 2;
				}

				if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
					float newAngle = ostrich.theta[ostrich.Body] - ostrich.rotateSpeed;
					//无碰撞检测
					ostrich.theta[ostrich.Body] = newAngle;
					if (camera->cameraNumber == 0) {
						glm::vec3 newAt = glm::rotate(glm::vec3(camera->at), glm::radians(-ostrich.rotateSpeed), (glm::vec3)camera->up);
						camera->at = glm::vec4(newAt, 1.0);
					}
					if (ostrich.move1state != 2) {
						ostrich.move1state = 1;
					}
				}
				else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
					if (ostrich.move1state == 1 && ostrich.move3state == 0 && ostrich.move4state == 0 &&
						glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_PRESS) ostrich.move1state = 2;
				}

				if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS) {
					camera->birdLoction = glm::vec3(transX, transY + 0.531, transZ);
					camera->fixed = false;
				}
			}
			//车辆控制
			else {
				if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
					car.frontSpeed = 0.020f;
					car.backSpeed = 0.015f;
					car.rotateZ = 10.0f;
					if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
						car.frontSpeed = 0.030f;
						car.backSpeed = 0.020f;
						car.rotateZ= 20.0f;
					}
				}
				else {
					car.frontSpeed = 0.012f;
					car.backSpeed = 0.008f;
					car.rotateWheel = 5.0f;
				}

				if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
					car.front = true; 
					float newX, newZ;
					newZ = carZ + (car.frontSpeed * cos(glm::radians(car.theta[0])));
					if (newZ >= 3.525) newZ = 3.525f;
					else if (newZ <= -3.466) newZ = -3.466f;
					newX = carX + (car.frontSpeed * sin(glm::radians(car.theta[0])));
					if (newX >= 3.525) newX = 3.525f;
					else if (newX <= -3.536) newX = -3.536f;
					//碰撞检测
					if (!checkCollision(newX, newZ, car.theta[0])) {
						carX = newX;
						carZ = newZ;
					}
					if (carZ > -2.87) {
						if (!start&&!over) {
							startTime = glfwGetTime();
							gameStart = glfwGetTime();
							currentWawa->setRotation(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
							mesh_npc1->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
							mesh_npc2->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
							start = true;
						}
					}
					if (carZ > 1.8626) {
						over = true;
						start = false;
					}
				}else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
					car.front = false;
					car.back = true;
					float newX, newZ;
					newZ = carZ - (car.backSpeed * cos(glm::radians(car.theta[0])));
					if (newZ >= 3.525) newZ = 3.525f;
					else if (newZ <= -3.466) newZ = -3.466f;
					newX = carX - (car.backSpeed * sin(glm::radians(car.theta[0])));
					if (newX >= 3.525) newX = 3.525f;
					else if (newX <= -3.536) newX = -3.536f;
					if (!checkCollision(newX, newZ, car.theta[0])) {
						carX = newX;
						carZ = newZ;
					}
					if (carZ > -2.87) {
						if (!start&&!over) {
							startTime = glfwGetTime();
							gameStart = glfwGetTime();
							currentWawa->setRotation(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
							mesh_npc1->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
							mesh_npc2->setRotation(glm::vec3(0.0f, 180.0f, 0.0f));
							start = true;
						}
					}
					if (carZ > 1.8626) {
						over = true;
						start = false;
					}
				}else {
					car.front = false;
					car.back = false;
				}
				if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
					car.left = true;
				}
				else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
					car.left = false;
					car.right = true;
				}
				else {
					car.left = false;
					car.right = false;
					car.theta[car.WheelY] = 0.0f;
				}

				if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
					car.cameraLoc = -1.0f;
					camera->carLoction2 = glm::vec3(carX - 1 * sin(glm::radians(car.theta[0])), carY + 0.531, carZ - 1 * cos(glm::radians(car.theta[0])));
					camera->at = glm::vec4(sin(glm::radians(car.theta[0])),-0.326f, cos(glm::radians(car.theta[0])), 1.0);
					camera->fixed = true;
				}
				
				cameraMove9();
			}
		}

		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			changeWawa(0);
		}
		else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			changeWawa(1);
		}
		else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
			changeWawa(2);
		}
		else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			changeWawa(3);
		}

		if (true) {
		if(ostrich.move5state == 0)ostrich.loading += ostrich.loadSpeed;
		
			if (ostrich.loading >= 1) {
				if (glfwGetKey(window, GLFW_KEY_TAB) != GLFW_PRESS) {
					ostrich.move5state = 1;
					ostrich.loading = 0;
				}
				else ostrich.loading = 1;
			}
			if (glfwGetKey(window, GLFW_KEY_B)) {
				int i;
				for (i = painter->getMeshNames().size() - 1; i >= 0; i--) {
					if (painter->getMeshNames()[i].compare("wawa")==0)
					{
						break;
					}
				}
			}
			if (glfwGetKey(window, GLFW_KEY_R) && (over||die)) {
				reset();
			}
		}
	}
}

//鸵鸟动作
void robotMove() {
	//扇动翅膀
	//判断行走动作是否激活
	if (ostrich.move1state == 1) {
		//判断翅膀角度是否达到最大（或最小）设定角度，若达到阈值则改变翅膀旋转方向，达到来回扇动翅膀的效果
		if (
			(ostrich.theta[ostrich.LeftWing] >= ostrich.wingMax && ostrich.wingDire == 1) || //是否达到最大角度并且扇动方向向外 
			(ostrich.theta[ostrich.LeftWing] <= ostrich.wingMin && ostrich.wingDire == -1)) //是否达到最小角度并且扇动方向向里
			ostrich.wingDire *= -1;	//改变翅膀扇动方向
		//根据扇动速度与扇动方向，改变翅膀角度
		ostrich.theta[ostrich.LeftWing] += (ostrich.wingSpeed * ostrich.wingDire);
		ostrich.theta[ostrich.RightWing] -= (ostrich.wingSpeed * ostrich.wingDire);
	}
	//判断是否为恢复状态
	else if (ostrich.move1state == 2) {
		//为了使恢复动作更为明显，同时又不至于太慢，设置循环次数为两次，使得恢复速度为扇动时的两倍。
		for (int i = 0; i < 2 && ostrich.theta[ostrich.LeftWing] > ostrich.wingMin; i++) {
			ostrich.theta[ostrich.LeftWing] += -ostrich.wingSpeed;
			ostrich.theta[ostrich.RightWing] += ostrich.wingSpeed;
		}
		//当动作复原后，动作控制变量的值更改为0，并且重置方向。
		if (ostrich.theta[ostrich.LeftWing] <= ostrich.wingMin) {
			ostrich.wingDire = 1;
			ostrich.move1state = 0;
			ostrich.theta[ostrich.LeftWing] = ostrich.theta[ostrich.RightWing] = ostrich.wingMin;
		}
	}

	//腿部动作
	if (ostrich.move2state == 1) {
		//左腿
		if (ostrich.isLeft2) {
			ostrich.theta[ostrich.LeftLeg] += (ostrich.Legspeed_2 * ostrich.d2_1);
			ostrich.theta[ostrich.LeftLowerLeg] += (ostrich.lowerLegSpeed * ostrich.d2_2);
			ostrich.theta[ostrich.LeftFoot] += (ostrich.footSpeed * ostrich.d2_3);
			if (ostrich.theta[ostrich.LeftLeg] >= ostrich.legMax2) {
				ostrich.isLeft2 = false;
				ostrich.d2_1 *= -1;
				ostrich.d2_2 *= -1;
				ostrich.d2_3 *= -1;
			}
			if (ostrich.theta[ostrich.LeftLeg] <= ostrich.legMin2) {
				ostrich.d2_1 *= -1;
				ostrich.d2_2 *= -1;
				ostrich.d2_3 *= -1;
			}
		}
		//右腿
		else {
			ostrich.theta[ostrich.RightLeg] += (ostrich.Legspeed_2 * ostrich.d2_1);
			ostrich.theta[ostrich.RightLowerLeg] += (ostrich.lowerLegSpeed * ostrich.d2_2);
			ostrich.theta[ostrich.RightFoot] += (ostrich.footSpeed * ostrich.d2_3);
			if (ostrich.theta[ostrich.RightLeg] >= ostrich.legMax2) {
				ostrich.isLeft2 = true;
				ostrich.d2_1 *= -1;
				ostrich.d2_2 *= -1;
				ostrich.d2_3 *= -1;
			}
			if (ostrich.theta[ostrich.RightLeg] <= ostrich.legMin2) {
				ostrich.d2_1 *= -1;
				ostrich.d2_2 *= -1;
				ostrich.d2_3 *= -1;
			}
		}
	}
	else if (ostrich.move2state == 2) {
		if (ostrich.theta[ostrich.LeftLeg] != ostrich.legMax2) {
			for (int i = 0; i < 2 && ostrich.theta[ostrich.LeftLeg] < ostrich.legMax2; i++) {
				ostrich.theta[ostrich.LeftLeg] += (ostrich.Legspeed_2);
				ostrich.theta[ostrich.LeftLowerLeg] += (ostrich.lowerLegSpeed);
				ostrich.theta[ostrich.LeftFoot] += -(ostrich.footSpeed);
			}
		}
		else if (ostrich.theta[ostrich.RightLeg] < ostrich.legMax2) {
			for (int i = 0; i < 2 && ostrich.theta[ostrich.RightLeg] != ostrich.legMax2; i++) {
				ostrich.theta[ostrich.RightLeg] += (ostrich.Legspeed_2);
				ostrich.theta[ostrich.RightLowerLeg] += (ostrich.lowerLegSpeed);
				ostrich.theta[ostrich.RightFoot] += -(ostrich.footSpeed);
			}
		}
		else {
			ostrich.move2state = 0;
			ostrich.theta[ostrich.LeftLeg] = ostrich.legMax2;
			ostrich.theta[ostrich.RightLeg] = ostrich.legMax2;
			ostrich.d2_1 = -1;
			ostrich.d2_2 = -1;
			ostrich.d2_3 = 1;
			ostrich.isLeft2 = !ostrich.isLeft2;
		}
	}

	if (ostrich.move3state == 1) {
		if (ostrich.theta[ostrich.Neck1] < ostrich.neck1Max3) {
			ostrich.theta[ostrich.Neck1] += (ostrich.neck1Speed3 * ostrich.d3_1);
			ostrich.theta[ostrich.Neck2] += (ostrich.neck2Speed3 * ostrich.d3_2);
			ostrich.theta[ostrich.Neck3] += (ostrich.neck3Speed3 * ostrich.d3_3);
			ostrich.theta[ostrich.Head] += (ostrich.headSpeed3 * ostrich.d3_4);
		}
	}
	else if (ostrich.move3state == 2) {
		for (int i = 0; i < 2 && ostrich.theta[ostrich.Neck1] > ostrich.neck1Min3; i++) {
			ostrich.theta[ostrich.Neck1] += (ostrich.neck1Speed3 * -ostrich.d3_1);
			ostrich.theta[ostrich.Neck2] += (ostrich.neck2Speed3 * -ostrich.d3_2);
			ostrich.theta[ostrich.Neck3] += (ostrich.neck3Speed3 * -ostrich.d3_3);
			ostrich.theta[ostrich.Head] += (ostrich.headSpeed3 * -ostrich.d3_4);
		}
		if (ostrich.theta[ostrich.Neck1] <= ostrich.neck1Min3) {
			if(ostrich.move1state == 0 && ostrich.move2state == 0) ostrich.move3state = 0;
			ostrich.theta[ostrich.Neck1] = ostrich.neck1Min3;
			ostrich.theta[ostrich.Neck2] = ostrich.neck2Min3;
			ostrich.theta[ostrich.Neck3] = ostrich.neck3Max3;
			ostrich.theta[ostrich.Head] = ostrich.headMax3;
		}
	}

	if (ostrich.move4state == 1) {
			if ((ostrich.theta[ostrich.Neck1] >= ostrich.neck1Max4 && ostrich.d4_1 == 1) || (ostrich.theta[ostrich.Neck1] <= ostrich.neck1Min4 && ostrich.d4_1 == -1)) {
				ostrich.d4_1 *= -1;
				ostrich.d4_2 *= -1;
				ostrich.d4_3 *= -1;
				ostrich.d4_4 *= -1;
			}
			ostrich.theta[ostrich.Neck1] += (ostrich.neck1Speed4 * ostrich.d4_1);
			ostrich.theta[ostrich.Neck2] += (ostrich.neck2Speed4 * ostrich.d4_2);
			ostrich.theta[ostrich.Neck3] += (ostrich.neck3Speed4 * ostrich.d4_3);
			ostrich.theta[ostrich.Head] += (ostrich.headSpeed4 * ostrich.d4_4);
		}
	else if (ostrich.move4state == 2) {
			for (int i = 0; i < 2 && ostrich.theta[ostrich.Neck1] > ostrich.neck1Min4; i++) {
				ostrich.theta[ostrich.Neck1] += (ostrich.neck1Speed4 * -1);
				ostrich.theta[ostrich.Neck2] += (ostrich.neck2Speed4 * -1);
				ostrich.theta[ostrich.Neck3] += (ostrich.neck3Speed4 * 1);
				ostrich.theta[ostrich.Head] += (ostrich.headSpeed4 * 1);
			}
			if (ostrich.theta[ostrich.Neck1] <= ostrich.neck1Min4) {
				if(ostrich.move1state == 0) ostrich.move4state = 0;
				ostrich.d4_1 *= 1;
				ostrich.d4_2 *= 1;
				ostrich.d4_3 *= -1;
				ostrich.d4_4 *= -1;
				ostrich.theta[ostrich.Neck1] = ostrich.neck1Min4;
				ostrich.theta[ostrich.Neck2] = ostrich.neck2Min4;
				ostrich.theta[ostrich.Neck3] = ostrich.neck3Max4;
				ostrich.theta[ostrich.Head] = ostrich.headMax4;
			}
		}

	if (ostrich.move5state == 1) {
		if (ostrich.d5 == 0) {
			if (ostrich.theta[ostrich.Head2] < 0) {
				ostrich.theta[ostrich.Head2] += ostrich.headSpeed5;
				if (ostrich.theta[ostrich.Head2] >= 0) {
					ostrich.move5state = 0;
					ostrich.theta[ostrich.Head2] = 0;
					ostrich.d5 = 1;
				}
			}
			else{
				ostrich.theta[ostrich.Head2] -= ostrich.headSpeed5;
				if (ostrich.theta[ostrich.Head2] <= 0) {
					ostrich.move5state = 0;
					ostrich.theta[ostrich.Head2] = 0;
					ostrich.d5 = -1;
				}
			}
		}
		else if(ostrich.move3state == 0){
			ostrich.theta[ostrich.Head2] += (ostrich.headSpeed5 * ostrich.d5);
			if (ostrich.theta[ostrich.Head2] * ostrich.d5 >= ostrich.headMax5) {
				ostrich.move5state = 0;
				ostrich.theta[ostrich.Head2] = ostrich.d5 * ostrich.headMax5;
				ostrich.d5 = 0;
			}
		}
	}
	
}

//汽车动作
void carMove() {
	// 如果当前控制的模型是鸵鸟，则退出函数
	if (modelIsOstrich) return;
	if (!die) {
		if (car.front) {
			car.theta[car.WheelZ] += car.rotateZ;
			if (car.theta[car.WheelY] > 360) car.theta[car.WheelY] -= 360.0f;
		}
		else if (car.back) {
			car.theta[car.WheelZ] -= car.rotateZ;
			if (car.theta[car.WheelY] < 0) car.theta[car.WheelY] += 360.0f;
		}

		if (car.left) {
			car.theta[car.WheelY] += car.rotateWheel;
			float newCarBodyTheta = car.theta[car.CarBody] + car.rotateCarBody;
			//碰撞检测
			if (!checkCollision(carX, carZ, newCarBodyTheta)) {
				car.theta[car.CarBody] = newCarBodyTheta;
			}
			if (camera->cameraNumber == 0) {
				//车辆旋转时，同时旋转相机
				camera->at = glm::vec4(sin(glm::radians(car.theta[0])), -0.326f, cos(glm::radians(car.theta[0])), 1.0);
			}
			if (car.theta[car.WheelY] > car.rotateWheelLimit) car.theta[car.WheelY] = car.rotateWheelLimit;
			if (car.theta[car.CarBody] > 360.0f) car.theta[car.CarBody] -= 360.0f;
		}else if (car.right) {
			car.theta[car.WheelY] -= car.rotateWheel;
			float newCarBodyTheta = car.theta[car.CarBody] - car.rotateCarBody;
			//碰撞检测
			if (!checkCollision(carX, carZ, newCarBodyTheta)) {
				car.theta[car.CarBody] = newCarBodyTheta;
			}
			if (camera->cameraNumber == 0) {
				camera->at = glm::vec4(sin(glm::radians(car.theta[0])), -0.326f, cos(glm::radians(car.theta[0])), 1.0);
			}
			if (car.theta[car.WheelY] < -car.rotateWheelLimit) car.theta[car.WheelY] = -car.rotateWheelLimit;
			if (car.theta[car.CarBody] < 0.0f) car.theta[car.CarBody] += 360.0f;
		}

		if (car.back) {
			carLeftLight->setAmbient(glm::vec4(5.0f, 0.0f, 0.0f, 1.0f));
			carRightLight->setAmbient(glm::vec4(5.0f, 0.0f, 0.0f, 1.0f));
		}
		else if (car.left) {
			carLeftLight->setAmbient(glm::vec4(5.0f, 0.0f, 0.0f, 1.0f));
			carRightLight->setAmbient(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
		}
		else if (car.right) {
			carRightLight->setAmbient(glm::vec4(5.0f, 0.0f, 0.0f, 1.0f));
			carLeftLight->setAmbient(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
		}
		else {
			carLeftLight->setAmbient(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
			carRightLight->setAmbient(glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));
		}
	}
}

bool isLeft = true;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	float tmp;
	glm::vec4 ambient;
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key)
		{
		case GLFW_KEY_U:
			if ((!start || over) && !modelIsOstrich ) {
				transX = carX;
				transZ = carZ;
				ostrich.theta[0] = car.theta[0];
				modelIsOstrich = true;
				if (camera->cameraNumber == 0) {
					camera->birdLoction = glm::vec3(transX, transY + 0.531, transZ);
					camera->fixed = false;
				}
			}
			break;
		case GLFW_KEY_I:
			if ((!start || over) && modelIsOstrich) {
				carX = transX;
				carZ = transZ;
				car.theta[0] = ostrich.theta[0];
				if (checkCollision(carX, carZ, car.theta[0])) break ;
				modelIsOstrich = false;
				if (camera->cameraNumber == 0) {
					camera->carLoction2 = glm::vec3(carX - 1 * sin(glm::radians(car.theta[0])), carY + 0.531, carZ - 1 * cos(glm::radians(car.theta[0])));
					camera->at = glm::vec4(sin(glm::radians(car.theta[0])), -0.326f, cos(glm::radians(car.theta[0])), 1.0);
					camera->fixed = true;
				}
			}
			break;
		case GLFW_KEY_TAB:
			if (camera->cameraNumber != 0) break;
			if (car.cameraLoc == -1.0f) car.cameraLoc = 0.2f;
			else if (car.cameraLoc == 0.2f) car.cameraLoc = -1.0f;
			break;
		case GLFW_KEY_M:
			defaultWawaMesh = !defaultWawaMesh;
			if (defaultWawaMesh) changeWawa(0);
			else changeWawa(3);
			break;
		case GLFW_KEY_K:
			currentWawa->setRotation(glm::vec3(0.0, 5.0, 0.0) + currentWawa->getRotation());
			break;
		case GLFW_KEY_L:
			currentWawa->setRotation(glm::vec3(0.0, -5.0, 0.0) + currentWawa->getRotation());
			break;
		}

		
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_PRESS) {
		
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			glm::vec4 lA = light->getAmbient();
			glm::vec4 lD = light->getAmbient();
			glm::vec4 lS = light->getAmbient();
			if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
				lA.x += (yoffset / 10); if (lA.x < 0.2) lA.x = 0.2; else if (lA.x > 2)lA.x = 2;
				lD.x += (yoffset / 10); if (lD.x < 0.2) lD.x = 0.2; else if (lD.x > 2)lD.x = 2;
				lS.x += (yoffset / 10); if (lS.x < 0.2) lS.x = 0.2; else if (lS.x > 2)lS.x = 2;
			}
			else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
				lA.y += (yoffset / 10); if (lA.y < 0.2) lA.y = 0.2; else if (lA.y > 2)lA.y = 2;
				lD.y += (yoffset / 10); if (lD.y < 0.2) lD.y = 0.2; else if (lD.y > 2)lD.y = 2;
				lS.y += (yoffset / 10); if (lS.y < 0.2) lS.y = 0.2; else if (lS.y > 2)lS.y = 2;
			}
			else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
				lA.z += (yoffset / 10); if (lA.z < 0.2) lA.z = 0.2; else if (lA.z > 2)lA.z = 2;
				lD.z += (yoffset / 10); if (lD.z < 0.2) lD.z = 0.2; else if (lD.z > 2)lD.z = 2;
				lS.z += (yoffset / 10); if (lS.z < 0.2) lS.z = 0.2; else if (lS.z > 2)lS.z = 2;
			}
			else {
				if (camera->fov >= 1.0f && camera->fov <= 89.0f) {
					camera->fov -= yoffset;
				}
				if (camera->fov <= 1.0f) camera->fov = 1.0f;
				else if (camera->fov >= 89.0f) camera->fov = 89.0f;
			}
			light->setAmbient(lA); lighMesh->setAmbient(lA);
			light->setDiffuse(lD); lighMesh->setDiffuse(lD);
			light->setSpecular(lS); lighMesh->setSpecular(lS);
		}
	}
	else {
		glm::vec3 pos = light->getTranslation();
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) pos.z += (yoffset / 2);
		else pos.z += (yoffset/5);
		if (pos.y > 4.1297) {
			if (pos.z < -4.192f) pos.z = -4.192f;
			else if (pos.z > 4.192f) pos.z = 4.192f;
		}
		else {
			if (pos.z < -4.192f) pos.z = 4.190f;
			else if (pos.z > 4.192f) pos.z = -4.190f;
		}
		light->setTranslation(pos);
		lighMesh->setTranslation(pos);
	}
}


int main(int argc, char** argv)
{
	system("mode con cols=93 lines=42");
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// 初始化GLFW库，必须是应用程序调用的第一个GLFW函数
	glfwInit();

	// 配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 配置窗口属性
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Squid Game", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);	
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Init mesh, shaders, buffer
	init();
	// 输出帮助信息
	printHelp();
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);
	srand(int(time(0)));
	while (!glfwWindowShouldClose(window))
	{
		display();
		camera->Inputs(window);
		mouse_input(window);
		key_input(window);
		wawaRotateControl();
		keepState1Time = rand()%2+2.5;
		if(modelIsOstrich) robotMove();
		else carMove();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanData();

	return 0;
}
