// Copyright 2017 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#ifndef __OPENSWE1R_SHADER_H__
#define __OPENSWE1R_SHADER_H__

#include <stdbool.h>

#include <GL/glew.h>

void PrintShaderLog(GLuint shader);
void PrintShaderProgramLog(GLuint program);
GLuint CreateShaderProgram(GLuint vertexShader, GLuint fragmentShader);
bool LinkShaderProgram(GLuint program);
GLuint CreateShader(const char* src, GLuint type);

#endif
