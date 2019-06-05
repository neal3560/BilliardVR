#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct Receive
{
	
	glm::vec3 head_pos;
	//int id;
	//glm::quat rotation;
	std::string to_string() {
		
		return std::to_string(head_pos[0]) + " " + std::to_string(head_pos[1]) + " " + std::to_string(head_pos[2]);
	}

	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP(head_pos.x, head_pos.y, head_pos.z)


};
