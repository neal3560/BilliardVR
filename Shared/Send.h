#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct Send
{
	
	glm::vec3 head_pos;
	//int id;
	//glm::quat rotation;


	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP(head_pos.x, head_pos.y, head_pos.z)


};
