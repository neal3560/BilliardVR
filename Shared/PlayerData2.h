#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct PlayerData2
{	
	glm::mat4 cuePose;

	float hit_volume;
	float cue_hit;
	float wall_hit;
	int pocketed;

	bool hold;

	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP
	(
		hit_volume, pocketed, cue_hit, wall_hit,
		hold,

		cuePose[0][0], cuePose[0][1], cuePose[0][2], cuePose[0][3],
		cuePose[1][0], cuePose[1][1], cuePose[1][2], cuePose[1][3],
		cuePose[2][0], cuePose[2][1], cuePose[2][2], cuePose[2][3],
		cuePose[3][0], cuePose[3][1], cuePose[3][2], cuePose[3][3]
	)

}; 
