#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

#define NUMBALL 11

struct Receive
{
	
	glm::vec3 head_pos;
	glm::vec3 ball_pos[NUMBALL];
	//int id;
	//glm::quat rotation;
	std::string to_string() {
		
		return std::to_string(ball_pos[0][0]) + " " + std::to_string(ball_pos[0][1]) + " " + std::to_string(ball_pos[0][2]);
	}

	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP(
		head_pos.x, head_pos.y, head_pos.z,
		// ball position
		ball_pos[0].x, ball_pos[0].y, ball_pos[0].z,
		ball_pos[1].x, ball_pos[1].y, ball_pos[1].z,
		ball_pos[2].x, ball_pos[2].y, ball_pos[2].z,
		ball_pos[3].x, ball_pos[3].y, ball_pos[3].z,
		ball_pos[4].x, ball_pos[4].y, ball_pos[4].z,
		ball_pos[5].x, ball_pos[5].y, ball_pos[5].z,
		ball_pos[6].x, ball_pos[6].y, ball_pos[6].z,
		ball_pos[7].x, ball_pos[7].y, ball_pos[7].z,
		ball_pos[8].x, ball_pos[8].y, ball_pos[8].z,
		ball_pos[9].x, ball_pos[9].y, ball_pos[9].z,
		ball_pos[10].x, ball_pos[10].y, ball_pos[10].z)


};
