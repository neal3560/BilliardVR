#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
#include "TexturedCube.h"

class Skybox : public TexturedCube
{
public:

  Skybox(const std::string dir);
  ~Skybox();

  void draw(unsigned int skyboxShader, const glm::mat4& p, const glm::mat4& v);

};
#endif
