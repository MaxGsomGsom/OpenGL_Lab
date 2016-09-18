#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW\glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <SOIL2.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h> 
using namespace Assimp;

#include <vector>
using namespace std;

#include "Helpers.h"


static int width = 800, height = 600;

int main(void)
{

#pragma region Init_libs

	// Инициалиация GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Инициалиация окна OpenGL
	window = glfwCreateWindow(width, height, "OpenGL_APP", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Инициалиация GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Режим ввода
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	//Очистка экрана
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	//Создание Vertex Array
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Компиляция шейдеров из файла
	GLuint programID = LoadShaders("VertexShader.gl", "FragmentShader.gl");

	// Вкл тест глубины
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

#pragma endregion

	// Получаем ID матриц из шейдера 
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Получаем ID текстуры из шейдера
	GLuint TextureID = glGetUniformLocation(programID, "MeshTexture");

	// Получаем ID источника света из шейдера
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	GLuint LightColorID = glGetUniformLocation(programID, "LightColor");
	GLuint LightPowerID = glGetUniformLocation(programID, "LightPower");

	GLuint AmbientColorPowerID = glGetUniformLocation(programID, "AmbientColorPower");
	GLuint SpecularColorPowerID = glGetUniformLocation(programID, "SpecularColorPower");

	//==============================================
	//Загрузка текстур
	GLuint TextureBall = SOIL_load_OGL_texture
		(
			"res/ball_1.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT);

	GLuint TextureCube = SOIL_load_OGL_texture
		(
			"res/cube_1.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT);

	GLuint TexturePlane = SOIL_load_OGL_texture
		(
			"res/plane_1.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT);

	//==============================================
	//Загрузка модели
	Importer importer;
	const aiScene* scene = importer.ReadFile("res/scene.obj", aiProcess_CalcTangentSpace |
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

	//массивы вершин
	vector<float>* vertices = new vector<float>[scene->mNumMeshes];
	vector<float>* uvs = new vector<float>[scene->mNumMeshes];
	vector<float>* normals = new vector<float>[scene->mNumMeshes];

	//Буферы вершин
	GLuint* vertexbuffer = new GLuint[scene->mNumMeshes];
	GLuint* uvbuffer = new GLuint[scene->mNumMeshes];
	GLuint* normalbuffer = new GLuint[scene->mNumMeshes];
	
	for (int m = 0; m < scene->mNumMeshes; m++) {
		aiMesh* mesh = scene->mMeshes[m];

		//заполняем массивы вершин
		for (int i = 0; i < mesh->mNumFaces; i++)
		{
			for (int k = 0; k < mesh->mFaces[i].mNumIndices; k++)
			{
				int indice = mesh->mFaces[i].mIndices[k];

				vertices[m].push_back(mesh->mVertices[indice].x);
				vertices[m].push_back(mesh->mVertices[indice].y);
				vertices[m].push_back(mesh->mVertices[indice].z);

				uvs[m].push_back(mesh->mTextureCoords[0][indice].x);
				uvs[m].push_back(mesh->mTextureCoords[0][indice].y);

				normals[m].push_back(mesh->mNormals[indice].x);
				normals[m].push_back(mesh->mNormals[indice].y);
				normals[m].push_back(mesh->mNormals[indice].z);
			}

		}

		//заполняем буферы вершин
		glGenBuffers(1, &vertexbuffer[m]);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[m]);
		glBufferData(GL_ARRAY_BUFFER, vertices[m].size() * sizeof(float), &vertices[m][0], GL_STATIC_DRAW);

		glGenBuffers(1, &uvbuffer[m]);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[m]);
		glBufferData(GL_ARRAY_BUFFER, uvs[m].size() * sizeof(float), &uvs[m][0], GL_STATIC_DRAW);

		glGenBuffers(1, &normalbuffer[m]);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[m]);
		glBufferData(GL_ARRAY_BUFFER, uvs[m].size() * sizeof(float), &normals[m][0], GL_STATIC_DRAW);

	}



	//==============================================

	do {
		// Очистка экрана
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Загрука шейдера
		glUseProgram(programID);

		//==============================================
		// Вычисление матрицы MVP
		ComputeMatricesFromInputs(window);
		mat4 ProjectionMatrix = GetProjectionMatrix();
		mat4 ViewMatrix = GetViewMatrix();
		mat4 ModelMatrix = mat4(1.0);
		mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Загрузка матриц в шейдер
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		//==============================================
		// Загрузка источника света в шейдер
		glUniform3f(LightID, 4, 4, 4);
		glUniform1f(LightPowerID, 50);
		glUniform3f(LightColorID, 1,1,1);
		glUniform3f(AmbientColorPowerID, 0.1, 0.1, 0.1);
		glUniform3f(SpecularColorPowerID, 0.3, 0.3, 0.3);

		// Загрузка текстуры в шейдер
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureBall);
		glUniform1i(TextureID, 0);

		// Загрузка вершин в шейдер
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer[0]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer[0]);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// отрисовка полигонов
		glDrawArrays(GL_TRIANGLES, 0, vertices[0].size());


		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		
		// Свопнуть буферы
		glfwSwapBuffers(window);
		glfwPollEvents();

	} //Проверка на закрытие окна
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	//==========================================
	// Очистка буферов
	for (int m = 0; m < scene->mNumMeshes; m++) {
		glDeleteBuffers(1, &vertexbuffer[m]);
		glDeleteBuffers(1, &uvbuffer[m]);
		glDeleteBuffers(1, &normalbuffer[m]);
	}
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Закрытие окна
	glfwTerminate();

	return 0;
}

