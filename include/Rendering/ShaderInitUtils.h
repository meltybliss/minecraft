#pragma once
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <string>
#include <fstream>
#include <sstream>


std::string ReadFile(const std::string& path);

unsigned int CompileShader(unsigned int type, const std::string& source);

unsigned int CreateShaderProgram(const std::string& vertPath, const std::string& fragPath);