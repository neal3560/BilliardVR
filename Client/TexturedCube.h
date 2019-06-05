#ifndef TEXTUREDCUBE_H
#define TEXTUREDCUBE_H

#include "Cube.h"
#include <string>

class TexturedCube : public Cube
{
public:

  TexturedCube(const std::string dir);
  ~TexturedCube();

  void draw(unsigned int shader, const glm::mat4& p, const glm::mat4& v);

  // These variables are needed for the shader program
  unsigned int cubeMap;
  unsigned int uProjection, uView;
};
#endif
