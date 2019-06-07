#version 410 core
// This is a sample fragment shader.

// Inputs to the fragment shader are the outputs of the same name from the vertex shader.
// Note that you do not have access to the vertex shader's default output, gl_Position.
in vec2 TC2;
in vec3 TC3;

in vec4 v;
in vec3 n;

uniform sampler2D texture2d;
uniform samplerCube texture3d;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

// rendering mode
uniform int mode;

// modelview used to process normal and vertex
uniform mat4 modelview;

const int numLights = 3; 
uniform vec4 lightposn[numLights]; // positions of lights 
uniform vec4 lightcolor[numLights]; // colors of lights
uniform int numused;               // number of lights used


// You can output many things. The first vec4 type output determines the color of the fragment
out vec4 fragColor;

vec4 ComputeLight (const in vec3 direction, const in vec4 lightcolor, const in vec3 normal, 
				   const in vec3 halfvec, const in vec4 mydiffuse, const in vec4 myspecular, 
				   const in float myshininess) {

        float nDotL = dot(normal, direction) ;         
        vec4 lambert = mydiffuse * lightcolor * max (nDotL, 0.0) ;  

        float nDotH = dot(normal, halfvec) ; 
        vec4 phong = myspecular * lightcolor * pow (max(nDotH, 0.0), myshininess) ; 

        vec4 retval = lambert + phong ; 
        return retval ;            
}

void main()
{
	if(mode == 0){
		fragColor = vec4(0.0f, 1.0f, 0.0f, 0.0f);
	}else if(mode == 1){
		// material
		vec4 diffuse = texture2D(texture_diffuse, TC2);
		vec4 specular = texture2D(texture_specular, TC2);
		vec4 ambient = diffuse * 0.2f;

		fragColor = ambient;

		// Compute eye direction
		const vec3 eyepos = vec3(0,0,0) ;
		vec4 vertex = modelview * v;
        vec3 mypos = vertex.xyz / vertex.w ;                // Dehomogenize current location 
        vec3 eyedirn = normalize(eyepos - mypos) ; 

        // Compute normal, needed for shading. 
        vec3 normal = normalize(mat3(transpose(inverse(modelview))) * n) ; 

		// loop through all lights
		for(int i = 0; i < numused; i++) {
			vec3 direction;
			if(lightposn[i].w == 0){
				direction = normalize (lightposn[i].xyz) ;
			} else {
				vec3 position = lightposn[i].xyz / lightposn[i].w ;
				direction = normalize (position - mypos) ; // no attenuation 
			}
			vec3 h = normalize (direction + eyedirn) ;  
			vec4 col = ComputeLight(direction, lightcolor[i], normal, h, diffuse, specular, 200) ;
			fragColor += col;
		}
	}else if(mode == 2){
		fragColor = texture(texture3d, TC3);
	}else if(mode == 3){
		fragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	}else if(mode == 4){
		fragColor = vec4(1.0f, 1.0f, 1.0f, 0.0f);
	}
}
