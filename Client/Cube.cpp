#include "Cube.h"

// Define the coordinates and indices needed to draw the cube. Note that it is not necessary
// to use a 2-dimensional array, since the layout in memory is the same as a 1-dimensional array.
// This just looks nicer since it's easy to tell what coordinates/indices belong where.
const GLfloat vertices[] = {
  -1.0f, 1.0f, -1.0f,
  -1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,

  -1.0f, -1.0f, 1.0f,
  -1.0f, -1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,
  -1.0f, 1.0f, -1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f,

  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,

  -1.0f, -1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, -1.0f, 1.0f,
  -1.0f, -1.0f, 1.0f,

  -1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, -1.0f,
  1.0f, 1.0f, 1.0f,
  1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, 1.0f,
  -1.0f, 1.0f, -1.0f,

  -1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, 1.0f,
  1.0f, -1.0f, -1.0f,
  1.0f, -1.0f, -1.0f,
  -1.0f, -1.0f, 1.0f,
  1.0f, -1.0f, 1.0f
};

const GLfloat normals[] = {
  0.0f, 0.0f, -1.0f,
  0.0f, 0.0f, -1.0f,
  0.0f, 0.0f, -1.0f,
  0.0f, 0.0f, -1.0f,
  0.0f, 0.0f, -1.0f,
  0.0f, 0.0f, -1.0f,

  -1.0f, 0.0f, 0.0f,
  -1.0f, 0.0f, 0.0f,
  -1.0f, 0.0f, 0.0f,
  -1.0f, 0.0f, 0.0f,
  -1.0f, 0.0f, 0.0f,
  -1.0f, 0.0f, 0.0f,

  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,
  1.0f, 0.0f, 0.0f,

  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 1.0f,

  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,
  0.0f, 1.0f, 0.0f,

  0.0f, -1.0f, 0.0f,
  0.0f, -1.0f, 0.0f,
  0.0f, -1.0f, 0.0f,
  0.0f, -1.0f, 0.0f,
  0.0f, -1.0f, 0.0f,
  0.0f, -1.0f, 0.0f,
};

Cube::Cube() {
  toWorld = glm::mat4(1.0f);

  // Create array object and buffers. Remember to delete your buffers when the object is destroyed!
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &vertexBuffer);
  glGenBuffers(1, &normalBuffer);

  // Bind the Vertex Array Object (VAO) first, then bind the associated buffers to it.
  // Consider the VAO as a container for all your buffers.
  glBindVertexArray(VAO);

  // Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
  // you want to draw, such as vertices, normals, colors, etc.
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  // glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
  // the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Enable the usage of layout location 0 (check the vertex shader to see what this is)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0,
    // This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 0. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
    3,
    // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
    GL_FLOAT, // What type these components are
    GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
    3 * sizeof(GLfloat),
    // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
    (GLvoid*)0);
  // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

  // Now bind a VBO to it as a GL_ARRAY_BUFFER. The GL_ARRAY_BUFFER is an array containing relevant data to what
  // you want to draw, such as vertices, normals, colors, etc.
  glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
  // glBufferData populates the most recently bound buffer with data starting at the 3rd argument and ending after
  // the 2nd argument number of indices. How does OpenGL know how long an index spans? Go to glVertexAttribPointer.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // Enable the usage of layout location 0 (check the vertex shader to see what this is)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1,
    // This first parameter x should be the same as the number passed into the line "layout (location = x)" in the vertex shader. In this case, it's 1. Valid values are 0 to GL_MAX_UNIFORM_LOCATIONS.
    3,
    // This second line tells us how any components there are per vertex. In this case, it's 3 (we have an x, y, and z component)
    GL_FLOAT, // What type these components are
    GL_FALSE, // GL_TRUE means the values should be normalized. GL_FALSE means they shouldn't
    3 * sizeof(GLfloat),
    // Offset between consecutive indices. Since each of our vertices have 3 floats, they should have the size of 3 floats in between
    (GLvoid*)0);
  // Offset of the first vertex's component. In our case it's 0 since we don't pad the vertices array with anything.

  // Unbind the currently bound buffer so that we don't accidentally make unwanted changes to it.
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // Unbind the VAO now so we don't accidentally tamper with it.
  // NOTE: You must NEVER unbind the element array buffer associated with a VAO!
  glBindVertexArray(0);
}

Cube::~Cube() {
  // Delete previously generated buffers. Note that forgetting to do this can waste GPU memory in a 
  // large project! This could crash the graphics driver due to memory leaks, or slow down application performance!
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &vertexBuffer);
  glDeleteBuffers(1, &normalBuffer);
}

void Cube::draw(GLuint shaderProgram, const glm::mat4& projection, const glm::mat4& view) {
  glUseProgram(shaderProgram);
  // Calculate the combination of the model and view (camera inverse) matrices
  glm::mat4 modelview = view * toWorld;
  // We need to calcullate this because modern OpenGL does not keep track of any matrix other than the viewport (D)
  // Consequently, we need to forward the projection, view, and model matrices to the shader programs
  // Get the location of the uniform variables "projection" and "modelview"
  glUniform1i(glGetUniformLocation(shaderProgram, "mode"), mode);
  uProjection = glGetUniformLocation(shaderProgram, "projection");
  uModelview = glGetUniformLocation(shaderProgram, "modelview");
  // Now send these values to the shader program
  glUniformMatrix4fv(uProjection, 1, GL_FALSE, &projection[0][0]);
  glUniformMatrix4fv(uModelview, 1, GL_FALSE, &modelview[0][0]);
  // Now draw the cube. We simply need to bind the VAO associated with it.
  glBindVertexArray(VAO);
  // Tell OpenGL to draw with triangles
  glDrawArrays(GL_TRIANGLES, 0, 3 * 2 * 6); // 3 vertices per triangle, 2 triangles per face, 6 faces
  // Unbind the VAO when we're done so we don't accidentally draw extra stuff or tamper with its bound buffers
  glBindVertexArray(0);
}

void Cube::update() {
  spin(1.0f);
}

void Cube::spin(float deg) {
  // If you haven't figured it out from the last project, this is how you fix spin's behavior
  toWorld = toWorld * glm::rotate(glm::mat4(1.0f), 1.0f / 180.0f * glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
}
