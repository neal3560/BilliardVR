#include "Skybox.h"

#include <GL/glew.h>
#include <iostream>
#include <vector>


Skybox::Skybox(const std::string dir) : TexturedCube(dir)
{
	mode = 2;
}

Skybox::~Skybox()
{
}

void Skybox::draw(unsigned skyboxShader, const glm::mat4& p, const glm::mat4& v)
{
  glDisable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glDepthMask(GL_FALSE);
  glUniform1i(glGetUniformLocation(skyboxShader, "mode"), mode);
  TexturedCube::draw(skyboxShader, p, glm::mat4(glm::mat3(v)));
  glDepthMask(GL_TRUE);
  glCullFace(GL_FRONT);
}
