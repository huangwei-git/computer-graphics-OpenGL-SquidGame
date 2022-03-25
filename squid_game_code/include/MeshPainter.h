#ifndef _MESH_PAINTER_H_
#define _MESH_PAINTER_H_

#include "TriMesh.h"
#include "Angel.h"

#include "Camera.h"

#include <vector>





class MeshPainter
{

public:
    MeshPainter();
    ~MeshPainter();

	float shadowLoc = -1.1;
    std::vector<std::string> getMeshNames();

    std::vector<TriMesh *> getMeshes();
    std::vector<openGLObject> getOpenGLObj();

	// 读取纹理文件
    void load_texture_STBImage(const std::string &file_name, GLuint& texture);
	//void load_texture_STBImages(const std::vector files, GLuint& texture);

	// 传递光线材质数据的
    void bindLightAndMaterial(TriMesh* mesh, openGLObject& object, Light* light, Camera* camera);

    void bindObjectAndData(TriMesh *mesh, openGLObject &object, const std::string &texture_image, const std::string &vshader, const std::string &fshader);

	// 添加物体
    void addMesh( TriMesh* mesh, const std::string &name, const std::string &texture_image, bool havaShadow, const std::string &vshader, const std::string &fshader );

	// 绘制物体
    void drawMesh(TriMesh* mesh, openGLObject &object, Light *light, Camera* camera);

	void shadowMapDraw(TriMesh* mesh, openGLObject& object, Light* light, Camera* camera, GLuint program);

	void shadowMapDraws(Light* light, Camera* camera, GLuint program);

	// 绘制多个物体
    void drawMeshes(Light *light, Camera* camera);

	// 清空数据
    void cleanMeshes();

private:
    std::vector<std::string> mesh_names;
    std::vector<TriMesh *> meshes;
    std::vector<openGLObject> opengl_objects;

};

#endif