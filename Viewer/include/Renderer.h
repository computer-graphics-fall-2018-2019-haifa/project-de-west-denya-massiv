#pragma once
#include "Scene.h"
#include "Line.h"
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>


/*
 * Renderer class.
 */
class Renderer
{
private:
	float *colorBuffer;
	float *zBuffer;
	int viewportWidth;
	int viewportHeight;
	int viewportX;
	int viewportY;

	void putPixel(int x, int y, const glm::vec3& color);
	void createBuffers(int viewportWidth, int viewportHeight);
	void drawLine(Line& line);
	void drawTriangle(Line lineA, Line lineB, Line lineC);

	GLuint glScreenTex;
	GLuint glScreenVtc;

	void createOpenGLBuffer();
	void initOpenGLRendering();
	void setXZeroToBeSmaller(Line & line, unsigned int &x0, unsigned int &y0, unsigned int &x1, unsigned int &y1);
	void drawStraightLine(unsigned int &y0, unsigned int &y1, unsigned int x0, unsigned int x1, glm::vec3 & lineColor);

public:
	Renderer(int viewportWidth, int viewportHeight, int viewportX = 0, int viewportY = 0);
	~Renderer();

	void Render(const Scene& scene);
	void SwapBuffers();
	void ClearColorBuffer(const glm::vec3& color);
	void SetViewport(int viewportWidth, int viewportHeight, int viewportX = 0, int viewportY = 0);


	// Add more methods/functionality as needed...
};
