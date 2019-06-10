#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

struct Sound {
	int hit_volume;
	MSGPACK_DEFINE_MAP
	(
		hit_volume
	)
};
