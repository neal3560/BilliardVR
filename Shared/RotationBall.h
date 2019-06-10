#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

#define NUMBALL 16

struct RotationBall
{
	glm::quat ball_rotation[NUMBALL];
	
	MSGPACK_DEFINE_MAP
	(
		// ball rotation
		ball_rotation[0].x, ball_rotation[0].y, ball_rotation[0].z, ball_rotation[0].w,
		ball_rotation[1].x, ball_rotation[1].y, ball_rotation[1].z, ball_rotation[1].w,
		ball_rotation[2].x, ball_rotation[2].y, ball_rotation[2].z, ball_rotation[2].w,
		ball_rotation[3].x, ball_rotation[3].y, ball_rotation[3].z, ball_rotation[3].w,
		ball_rotation[4].x, ball_rotation[4].y, ball_rotation[4].z, ball_rotation[4].w,
		ball_rotation[5].x, ball_rotation[5].y, ball_rotation[5].z, ball_rotation[5].w,
		ball_rotation[6].x, ball_rotation[6].y, ball_rotation[6].z, ball_rotation[6].w,
		ball_rotation[7].x, ball_rotation[7].y, ball_rotation[7].z, ball_rotation[7].w,
		ball_rotation[8].x, ball_rotation[8].y, ball_rotation[8].z, ball_rotation[8].w,
		ball_rotation[9].x, ball_rotation[9].y, ball_rotation[9].z, ball_rotation[9].w,
		ball_rotation[10].x, ball_rotation[10].y, ball_rotation[10].z, ball_rotation[10].w,
		ball_rotation[11].x, ball_rotation[11].y, ball_rotation[11].z, ball_rotation[11].w,
		ball_rotation[12].x, ball_rotation[12].y, ball_rotation[12].z, ball_rotation[12].w,
		ball_rotation[13].x, ball_rotation[13].y, ball_rotation[13].z, ball_rotation[13].w,
		ball_rotation[14].x, ball_rotation[14].y, ball_rotation[14].z, ball_rotation[14].w,
		ball_rotation[15].x, ball_rotation[15].y, ball_rotation[15].z, ball_rotation[15].w
	)
};