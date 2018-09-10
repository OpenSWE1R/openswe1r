// Copyright 2017 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#include <stdlib.h>
#include <stdio.h>

#include "shader.h"

void PrintShaderLog(GLuint shader) {
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0) {
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
	return;
}

void PrintShaderProgramLog(GLuint program) {
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0) {
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}

	return;
}

GLuint CreateShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	return program;
}

bool LinkShaderProgram(GLuint program) {
	glLinkProgram(program);
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	return isLinked != GL_FALSE;
}

GLuint CreateShader(const char* source, GLuint type) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	return shader;
}
