#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct ClientData2 
{
	glm::mat4 cuePose;
	glm::vec4 cue_point;

	bool hold;
	bool hit;
	MSGPACK_DEFINE_MAP
	(
		cuePose[0][0], cuePose[0][1], cuePose[0][2], cuePose[0][3],
		cuePose[1][0], cuePose[1][1], cuePose[1][2], cuePose[1][3],
		cuePose[2][0], cuePose[2][1], cuePose[2][2], cuePose[2][3],
		cuePose[3][0], cuePose[3][1], cuePose[3][2], cuePose[3][3],

		// cue
		cue_point.x, cue_point.y, cue_point.z, cue_point.w,
		hold, hit
	)
};
