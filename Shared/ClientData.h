#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct ClientData
{
	
	glm::vec3 head_pos;
	glm::quat head_rot;
	glm::vec3 controller_location[2];
	glm::quat controller_rotation[2];
	bool hit;

	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP
	(
		head_pos.x, head_pos.y, head_pos.z, 
		head_rot.x, head_rot.y, head_rot.z, head_rot.w,
		controller_location[0].x, controller_location[0].y, controller_location[0].z,
		controller_location[1].x, controller_location[1].y, controller_location[1].z,
		controller_rotation[0].x, controller_rotation[0].y, controller_rotation[0].z, controller_rotation[0].w,
		controller_rotation[1].x, controller_rotation[1].y, controller_rotation[1].z, controller_rotation[1].w,
		hit
	)
};
