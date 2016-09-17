#pragma once
#include <GLFW\glfw3.h>

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);

void ComputeMatricesFromInputs(GLFWwindow* window);
mat4 GetViewMatrix();
mat4 GetProjectionMatrix();