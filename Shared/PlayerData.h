#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct PlayerData
{
	int id;
	int state;
	int cur_round;
	glm::mat4 headPose;
	glm::mat4 controllerPose[2];

	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP
	(
		id,
		state,
		cur_round,
		headPose[0][0], headPose[0][1], headPose[0][2], headPose[0][3],
		headPose[1][0], headPose[1][1], headPose[1][2], headPose[1][3],
		headPose[2][0], headPose[2][1], headPose[2][2], headPose[2][3],
		headPose[3][0], headPose[3][1], headPose[3][2], headPose[3][3],

		controllerPose[0][0][0], controllerPose[0][0][1], controllerPose[0][0][2], controllerPose[0][0][3],
		controllerPose[0][1][0], controllerPose[0][1][1], controllerPose[0][1][2], controllerPose[0][1][3],
		controllerPose[0][2][0], controllerPose[0][2][1], controllerPose[0][2][2], controllerPose[0][2][3],
		controllerPose[0][3][0], controllerPose[0][3][1], controllerPose[0][3][2], controllerPose[0][3][3],

		controllerPose[1][0][0], controllerPose[1][0][1], controllerPose[1][0][2], controllerPose[1][0][3],
		controllerPose[1][1][0], controllerPose[1][1][1], controllerPose[1][1][2], controllerPose[1][1][3],
		controllerPose[1][2][0], controllerPose[1][2][1], controllerPose[1][2][2], controllerPose[1][2][3],
		controllerPose[1][3][0], controllerPose[1][3][1], controllerPose[1][3][2], controllerPose[1][3][3]
	)

}; 