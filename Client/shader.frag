#version 410 core
// This is a sample fragment shader.

// Inputs to the fragment shader are the outputs of the same name from the vertex shader.
// Note that you do not have access to the vertex shader's default output, gl_Position.
in vec2 TC2;
in vec3 TC3;

uniform sampler2D texture2d;
uniform samplerCube texture3d;
uniform int mode;


// You can output many things. The first vec4 type output determines the color of the fragment
out vec4 fragColor;

void main()
{
	if(mode == 0){
		fragColor = vec4(0.0f, 1.0f, 0.0f, 0.0f);
	}else if(mode == 1){
		fragColor = texture2D(texture2d, TC2);
	}else if(mode == 2){
		fragColor = texture(texture3d, TC3);
	}else if(mode == 3){
		fragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}else if(mode == 4){
		fragColor = vec4(1.0f, 1.0f, 1.0f, 0.0f);
	}
}
