#include <rpc/client.h>
#include "glm/glm.hpp"
#include <glm/gtx/string_cast.hpp>

#define NUMBALL 16

struct OnTableBall
{
	bool on_table[NUMBALL];
	// rpc Macro to generate serialize code for the struct (Note: for glm object, manually specify x,y,z,w)
	MSGPACK_DEFINE_MAP(
		on_table[0], on_table[1], on_table[2], on_table[3], on_table[4], 
		on_table[5], on_table[6], on_table[7], on_table[8], on_table[9], 
		on_table[10], on_table[11], on_table[12], on_table[13], on_table[14], on_table[15]
	)


};