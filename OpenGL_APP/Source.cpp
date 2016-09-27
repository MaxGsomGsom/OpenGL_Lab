#pragma region Includes

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
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

#pragma endregion

static int width = 1000, height = 1000;


void DrawMesh(GLint vertexbuffer, GLint uvbuffer, GLint normalbuffer, int verticesSize) {
	// Загрузка вершин в шейдер
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// отрисовка полигонов
	glDrawArrays(GL_TRIANGLES, 0, verticesSize);


	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}


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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	//Создание Vertex Array
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);


	// Вкл тест глубины
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);


#pragma endregion

#pragma region Shaders

	// Компиляция шейдеров из файла
	GLuint programID = LoadShaders("VertexShader.gl", "FragmentShader.gl");
	GLuint programID_RTT = LoadShaders("VertexShader.gl", "FragmentShaderRTT.gl");

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

	//=================================

	// Получаем ID матриц из шейдера 
	GLuint MatrixID_RTT = glGetUniformLocation(programID_RTT, "MVP");
	GLuint ViewMatrixID_RTT = glGetUniformLocation(programID_RTT, "V");
	GLuint ModelMatrixID_RTT = glGetUniformLocation(programID_RTT, "M");

	// Получаем ID текстуры из шейдера
	GLuint TextureID_RTT = glGetUniformLocation(programID_RTT, "MeshTexture");

	// Получаем ID источника света из шейдера
	GLuint LightID_RTT = glGetUniformLocation(programID_RTT, "LightPosition_worldspace");
	GLuint LightColorID_RTT = glGetUniformLocation(programID_RTT, "LightColor");
	GLuint LightPowerID_RTT = glGetUniformLocation(programID_RTT, "LightPower");

	GLuint AmbientColorPowerID_RTT = glGetUniformLocation(programID_RTT, "AmbientColorPower");
	GLuint SpecularColorPowerID_RTT = glGetUniformLocation(programID_RTT, "SpecularColorPower");

	//==============================================
#pragma endregion

#pragma region Model_Loading
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

#pragma endregion

#pragma region FrameBuffer
	//==============================================

	// Создаем фреймбуфер
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// Создаем пустую текстуру
	GLuint renderedTexture;
	glGenTextures(1, &renderedTexture);
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Фильтрация текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Буфер глубины
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// ПРикрепляем текстуру к фреймбуферу
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);

	// ПРоверяем фреймбуфер
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

#pragma endregion

	double xx = 0;
	double yy = 0;
	double zz = 0;

	do {
		vec3 lightPos = vec3(5+xx, 5+yy, 5);
		float lightPower = 50;

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
			xx += 0.1;
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
			xx -= 0.1;
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
			yy += 0.1;
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
			yy -= 0.1;
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
			zz += 0.1;
		if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
			zz -= 0.1;

		//==========================================
		//==========================================
		//==========================================

		// Рендерить изображение в буффер
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
		glViewport(0, 0, width, height);

		// Очистка экрана
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		// Загрука шейдера
		glUseProgram(programID_RTT);

		//==============================================
		// Вычисление матрицы MVP
		ComputeMatricesFromInputs(window);
		mat4 ProjectionMatrix_RTT = GetProjectionMatrix();
		mat4 ViewMatrix_RTT = GetViewMatrixReflection();
		mat4 ModelMatrix_RTT = mat4(1.0);
		mat4 MVP_RTT = ProjectionMatrix_RTT * ViewMatrix_RTT * ModelMatrix_RTT;

		// Загрузка матриц в шейдер
		glUniformMatrix4fv(MatrixID_RTT, 1, GL_FALSE, &MVP_RTT[0][0]);
		glUniformMatrix4fv(ModelMatrixID_RTT, 1, GL_FALSE, &ModelMatrix_RTT[0][0]);
		glUniformMatrix4fv(ViewMatrixID_RTT, 1, GL_FALSE, &ViewMatrix_RTT[0][0]);

		//==============================================
		// Загрузка источника света в шейдер
		glUniform3f(LightID_RTT, lightPos.x, lightPos.y, lightPos.z);
		glUniform1f(LightPowerID_RTT, lightPower);
		glUniform3f(LightColorID_RTT, 1, 1, 1);
		glUniform3f(AmbientColorPowerID_RTT, 0.15, 0.15, 0.15);
		glUniform3f(SpecularColorPowerID_RTT, 0.3, 0.3, 0.3);

		// Загрузка текстуры в шейдер
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureBall);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureCube);

		//рисуем шары
		glUniform1i(TextureID_RTT, 0);
		int m = 0;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());
		m = 3;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());
		m = 4;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());
		m = 5;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());

		//куб
		glUniform1i(TextureID_RTT, 1);
		m = 2;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());


		//==========================================
		//==========================================
		//==========================================


		// Рендерить на экран
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, width, height);

		// Очистка экрана
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.7f, 0.7f, 1.0f, 0.0f);

		// Загрузка шейдера
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
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform1f(LightPowerID, lightPower);
		glUniform3f(LightColorID, 1, 1, 1);
		glUniform3f(AmbientColorPowerID, 0.15, 0.15, 0.15);
		glUniform3f(SpecularColorPowerID, 0.3, 0.3, 0.3);

		// Загрузка текстуры в шейдер
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureBall);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TextureCube);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, renderedTexture); //полученная текстура


		//рисуем шары
		glUniform1i(TextureID, 0);
		m = 0;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());
		m = 3;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());
		m = 4;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());
		m = 5;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());

		//плоскость
		glUniform1i(TextureID, 2);
		m = 1;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());

		//куб
		glUniform1i(TextureID, 1);
		m = 2;
		DrawMesh(vertexbuffer[m], uvbuffer[m], normalbuffer[m], vertices[m].size());


		//==========================================
		//==========================================
		//==========================================

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
	glDeleteProgram(programID_RTT);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Закрытие окна
	glfwTerminate();

	return 0;
}
