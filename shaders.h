// Copyright 2017 OpenSWE1R Maintainers
// Licensed under GPLv2 or any later version
// Refer to the included LICENSE.txt file.

#ifndef __OPENSWE1R_SHADERS_H__
#define __OPENSWE1R_SHADERS_H__

static const char* VertexShader1Texture =
"#version 330\n"
"\n"
"uniform mat4 projectionMatrix;\n"
"\n"
"uniform vec3 clipScale;\n"
"uniform vec3 clipOffset;\n"
"\n"
"in vec4 positionIn;\n"
"in vec4 diffuseIn;\n"
"in vec4 specularIn;\n"
"in vec2 uv0In;\n"
"\n"
"out vec4 diffuse;\n"
"out vec4 specular;\n"
"out vec2 uv0;\n"
"\n"
"void main() {\n"
"  gl_Position = vec4(positionIn.xyz * clipScale + clipOffset, 1.0);\n"
"  gl_Position /= positionIn.w;\n"
"  gl_Position.y = -gl_Position.y;\n"
"  diffuse = diffuseIn.bgra;\n"
"  specular = specularIn.bgra;\n"
"  uv0 = uv0In;\n"
"}";

static const char* FragmentShader1Texture =
"#version 330\n"
"\n"
"uniform sampler2D tex0;\n"
"uniform bool alphaTest;\n"
"uniform int fogMode;\n"
"uniform float fogStart;\n"
"uniform float fogEnd;\n"
"uniform vec3 fogColor;\n"
"\n"
"in vec4 diffuse;\n"
"in vec4 specular;\n"
"in vec2 uv0;\n"
"\n"
"out vec4 color;\n"
#if 0
"\n"
"uniform sampler2D tex0;\n"
#endif
"\n"
"void main() {\n"
"  color = texture(tex0, uv0);\n"
"  color *= diffuse;\n"
"  if (alphaTest && !(int(round(color.a * 255.0)) != 0)) { discard; }\n"
"  if (fogMode == 0) {\n" // D3DFOG_NONE
"    color.rgb = color.rgb;\n"
"  } else if (fogMode == 3) {\n" // D3DFOG_LINEAR
"    float fogVisibility = (fogEnd - gl_FragCoord.z / gl_FragCoord.w) / (fogEnd - fogStart);\n"
"    fogVisibility = clamp(fogVisibility, 0.0, 1.0);\n"
"    color.rgb = mix(fogColor, color.rgb, fogVisibility);\n"
"  } else {\n" // Unknown fog mode; Signal error by coloring primitive green
"    color.rgb = vec3(0.0, 1.0, 0.0);\n"
"  }\n"
"}\n";

#endif
