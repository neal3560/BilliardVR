#ifndef CUBE_H
#define CUBE_H

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
// Use of degrees is deprecated. Use radians instead.
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Cube {
public:
  Cube();
  ~Cube();

  glm::mat4 toWorld;

  void draw(GLuint shaderProgram, const glm::mat4& projection, const glm::mat4& view);
  void update();
  void spin(float);

  // These variables are needed for the shader program
  GLuint vertexBuffer, normalBuffer, VAO;
  GLuint uProjection, uModelview;
  int mode;
};

#endif
