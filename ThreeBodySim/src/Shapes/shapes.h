#pragma once

GLuint getCubeList(float BOXSIZE);
GLuint getSphereList(float radius, int slices, int stacks);
GLuint getAxesList(const GLfloat axesWidth);
GLuint getConeList(float radBot, float radTop, float height, int slices, int stacks,bool top, bool bot);