﻿// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "rpc/server.h"
#include <string>
#include <iostream>
#include <vector>
#include <math.h>

// Shared struct
#include "LocationBall.h"
#include "RotationBall.h"
#include "OnTableBall.h"
#include "PlayerData.h"
#include "PlayerData2.h"
#include "ClientData.h"
#include "ClientData2.h"

#include <ctime>


using std::string;
using namespace glm;
using namespace std;

#define PORT 3560
#define NUMBALL 16
#define MID 0.68f
#define RADIUS 0.033f
#define POCKET_RADIUS 0.07f
#define INTERVAL 5
#define ENERGY_LOST 0.65f
#define F 0.3f
#define G 10.0f

#define VOLUME_OFFSET 0.2f

/*game state:
* 0 not start
* 1 player normal hit
* 2 player place cue
* 3 ball moving
* 4 game over
*/
int num_player;
int cur_round;
int state;       

int selected;   // -1 for not selected; 0 for first player 1-7; 1 for first player 9-15
bool foul;
bool lose;
bool pocketed;
bool from3;
bool first_collision;

mat4 headPoses[2];
mat4 handPoses[2][2];
mat4 cuePoses[2]; 

clock_t time_record[2];
vec3 cue_position[2];
vec3 cue_position_pre[2];
vec3 cue_velocity[2];

bool hold[2];
bool hit[2];

//sound effect
float hit_volume[2];
int is_pocketed[2];
float cue_hit_vel[2];
float hit_wall[2];

const vec3 ball_pos_init[NUMBALL] = 
{ 
	vec3(0.0f, RADIUS, MID),                          // cue ball
	vec3(0.0f, RADIUS, -MID),		                  // 1
	vec3(RADIUS, RADIUS, -MID - 1.732f * RADIUS),     // 2
	vec3(-RADIUS, RADIUS, -MID - 1.732f * RADIUS),
	vec3(0.0f, RADIUS, -MID - 3.364f * RADIUS),		  // 3
	vec3(RADIUS * 2, RADIUS, -MID - 3.364f * RADIUS),
	vec3(-RADIUS * 2, RADIUS, -MID - 3.364f * RADIUS),
	vec3(RADIUS, RADIUS, -MID - 5.196f * RADIUS),	  // 4
	vec3(-RADIUS, RADIUS, -MID - 5.196f * RADIUS),
	vec3(RADIUS * 3, RADIUS, -MID - 5.196f * RADIUS),
	vec3(-RADIUS * 3, RADIUS,  -MID - 5.196f * RADIUS),
	vec3(0.0f, RADIUS, -MID - 6.928f * RADIUS),       // 5
	vec3(-RADIUS * 2, RADIUS, -MID - 6.928f * RADIUS),
	vec3(RADIUS * 2, RADIUS, -MID - 6.928f * RADIUS),
	vec3(-RADIUS * 4, RADIUS, -MID - 6.928f * RADIUS),
	vec3(RADIUS * 4, RADIUS, -MID - 6.928f * RADIUS),
};

vec2 pocket_location[6] = 
{
	vec2(-MID, -MID * 2),
	vec2(MID, -MID * 2),
	vec2(-MID - 0.015f, 0.0f),
	vec2(MID + 0.015f, 0.0f),
	vec2(-MID, MID * 2),
	vec2(MID, MID * 2),
};

vec3 ball_pos[NUMBALL];
vec3 ball_velocity[NUMBALL];
quat ball_rotation[NUMBALL];
bool on_table[NUMBALL];

void initialize() {
	for (int i = 0; i < NUMBALL; i++) {
		ball_pos[i] = ball_pos_init[i];
		ball_velocity[i] = vec3(0.0f);
		ball_rotation[i] = angleAxis(0.0f, vec3(1.0f, 0.0f, 0.0f));
		on_table[i] = true;
	}
	cur_round = 0;
	selected = -1;
	foul = false;
	lose = false;
	first_collision = false;
	pocketed = false;
	from3 = false;
}

bool check_cue_hit() {
	for (int i = 0; i < 2; i++) {
		if (hit[i] && cur_round == i) {
			// check_if_hit
			vec3 hit_direction = normalize(vec3(cue_position[i]) - vec3(cue_position_pre[i]));
			vec3 in = ball_pos[0] - cue_position_pre[i];
			vec3 out = cue_position[i] - ball_pos[0];
			float distance = length(in - dot(in, hit_direction) * hit_direction);
			bool same_direction = dot(in, hit_direction) * dot(out, hit_direction) > 0;
			if (length(cue_position[i] - ball_pos[0]) <= RADIUS || (same_direction && distance <= RADIUS)) {
				float hit_distance = length(vec3(cue_position_pre[i]) - ball_pos[0]) - RADIUS;
				vec3 hit_position = cue_position_pre[i] + hit_distance * hit_direction;
				vec3 center_direction = ball_pos[0] - hit_position;
				center_direction = normalize(center_direction);
				ball_velocity[0] = dot(cue_velocity[i], center_direction) * center_direction;
				// sound
				cue_hit_vel[0] = length(ball_velocity[0]);
				cue_hit_vel[1] = length(ball_velocity[0]);
				return true;
			}
		}
	}
	return false;
}

void check_ball_hit() {
	// ball collision
	vector<int> collision;
	// detect collision
	for (int i = 0; i < NUMBALL - 1; i++) {
		if (on_table[i]) {
			for (int j = i + 1; j < NUMBALL; j++) {
				if (on_table[j]) {
					float overlap = 2 * RADIUS - length(ball_pos[i] - ball_pos[j]);
					if (overlap >= 0) {
						if (i == 0) {
							if (!first_collision) {
								first_collision = true;
								if (selected == -1) {
									if (j == 8) {
										foul = true;
									}
								}
								else if (cur_round == selected) {
									bool cleaned = true;
									for (int k = 1; k <= 7; k++) {
										if (on_table[k])
											cleaned = false;
									}
									if (j > 8 || (j == 8 && !cleaned)) {
										foul = true;
									}
								} else if(cur_round != selected) {
									bool cleaned = true;
									for (int k = 9; k <= 15; k++) {
										if (on_table[k])
											cleaned = false;
									}
									if (j < 8 || (j == 8 && !cleaned)) {
										foul = true;
									}
								}
							}
						}
						// fix overlap
						vec3 collide_direction = normalize(ball_pos[i] - ball_pos[j]);
						ball_pos[i] += overlap / 2 * collide_direction;
						ball_pos[j] -= overlap / 2 * collide_direction;
						collision.push_back(i);
						collision.push_back(j);
					}
				}
			}
		}
		
	}

	// redistribute volocity
	for (vector<int>::iterator it = collision.begin(); it != collision.end(); it += 2) {
		vec3 collide_direction = normalize(ball_pos[*it] - ball_pos[*(it + 1)]);
		// decompose ball velocity along collide direction
		vec3 velocity_A_1 = dot(ball_velocity[*it], collide_direction) * collide_direction;
		vec3 velocity_A_2 = ball_velocity[*it] - velocity_A_1;
		vec3 velocity_B_1 = dot(ball_velocity[*(it + 1)], collide_direction) * collide_direction;
		vec3 velocity_B_2 = ball_velocity[*(it + 1)] - velocity_B_1;
		// exchange velocity along collide direction
		ball_velocity[*it] = velocity_B_1 + velocity_A_2;
		ball_velocity[*(it + 1)] = velocity_A_1 + velocity_B_2;
		// hit sound effect
		hit_volume[0] += length(velocity_A_1 - velocity_B_1);
		hit_volume[1] += length(velocity_A_1 - velocity_B_1);
	}
	cout << "check wall and ground collision" << endl;
	// table and wall collision
	for (int i = 0; i < NUMBALL; i++) {
		// table
		if (ball_pos[i].y < RADIUS) {
			ball_pos[i].y = RADIUS;
			ball_velocity[i].y = -ENERGY_LOST * ball_velocity[i].y;
			if (ball_velocity[i].y < 0.3f) {
				cout << "i am 0" << endl;
				ball_velocity[i].y = 0.0f;
			}
		}

		// left side
		if (ball_pos[i][0] - RADIUS <= -MID) {
			hit_wall[0] += -ball_velocity[i][0];
			hit_wall[1] += -ball_velocity[i][0];
			ball_pos[i][0] = RADIUS - MID;
			ball_velocity[i][0] = -ball_velocity[i][0] * ENERGY_LOST;
			
		}
		// right side
		else if (ball_pos[i][0] + RADIUS >= MID) {
			hit_wall[0] += ball_velocity[i][0];
			hit_wall[1] += ball_velocity[i][0];
			ball_pos[i][0] = -RADIUS + MID;
			ball_velocity[i][0] = -ball_velocity[i][0] * ENERGY_LOST;
		}

		// front side
		if (ball_pos[i][2] - RADIUS <= -MID * 2) {
			hit_wall[0] += -ball_velocity[i][2];
			hit_wall[1] += -ball_velocity[i][2];
			ball_pos[i][2] = RADIUS - MID * 2;
			ball_velocity[i][2] = -ball_velocity[i][2] * ENERGY_LOST;
		}
		// rear side
		else if (ball_pos[i][2] + RADIUS >= MID * 2) {
			hit_wall[0] += ball_velocity[i][2];
			hit_wall[1] += ball_velocity[i][2];
			ball_pos[i][2] = MID * 2 - RADIUS;
			ball_velocity[i][2] = -ball_velocity[i][2] * ENERGY_LOST;
		}
	}
}

void check_pocketed() {
	for (int i = 0; i < NUMBALL; i++) {
		if (on_table[i]){
			// check whether pocketed
			for (int j = 0; j < 6; j++) {
				float dist_sq = pow(ball_pos[i].x - pocket_location[j].x, 2) + pow(ball_pos[i].z - pocket_location[j].y, 2);
				if (dist_sq <= pow(POCKET_RADIUS, 2)) {
					on_table[i] = false;
					int sound_mode = 0;                   //1 for good, -1 for bad
					// cue ball pocketed
					if (i == 0) {
						foul = true;
						sound_mode = -1;
					}
					// solid ball
					else if (i > 0 && i < 8) {
						// not yet selecte side
						sound_mode = 1;
						if (selected == -1) {
							pocketed = true;
							if (cur_round == 0) {
								selected = 0;
							}
							else {
								selected = 1;
							}
						}
						else if (selected == cur_round) {
							pocketed = true;
						}
					}
					// black 8 pocketed: game is over
					else if (i == 8) {
						state = 4;
						if (selected == -1) {
							lose = true;
						}
						else if (cur_round == selected) {
							for (int k = 1; k <= 7; k++) {
								if (on_table[k]) {
									lose = true;
								}
							}
						}
						else if (cur_round != selected) {
							for (int k = 9; k <= 15; k++) {
								if (on_table[k]) {
									lose = true;
								}
							}
						}
						// TODO: add win/lose sound
						if (lose) {
							sound_mode = -1;
						}
						else {
							sound_mode = 1;
						}
					}
					// strip ball
					else if ( i > 8 && i <= 15) {
						sound_mode = 1;
						// not yet select side
						pocketed = true;
						if (selected == -1) {
							if (cur_round == 0) {
								selected = 1;
							}
							else {
								selected = 0;
							}
						}
						else if (selected != cur_round) {
							pocketed = true;
						}
					}

					// set sound effect
					if (sound_mode == 1) {
						is_pocketed[0] = 1;
						is_pocketed[1] = 1;
					}
					else if(sound_mode == -1){
						is_pocketed[0] = -1;
						is_pocketed[1] = -1;
					}
				}
			}
		}
	}
}

bool check_all_stop() {
	for (int i = 0; i < NUMBALL; i++) {
		if (on_table[i] && length(ball_velocity[i]) > 0.0f) {
			return false;
		}
	}
	return true;
}



void update_pos() {
	for (int i = 0; i < NUMBALL; i++) {
		// change location
		ball_pos[i] += INTERVAL * 0.001f * ball_velocity[i];
		// change rotation
		vec3 direction = vec3(ball_velocity[i].x, 0.0f, ball_velocity[i].z);
		float angle = INTERVAL / RADIUS * 0.001f * length(direction);
		if (angle > 0) {
			vec3 axis = normalize(cross(vec3(0.0f, 1.0f, 0.0f), direction));
			ball_rotation[i] = angleAxis(angle, axis) * ball_rotation[i];
		}
		// change velocity
		if (ball_pos[i].y > RADIUS) {
			ball_velocity[i].y -= G * INTERVAL * 0.001f;
		}
		else if (ball_velocity[i].y == 0 && ball_pos[i].y == RADIUS) {
			vec3 new_velocity = ball_velocity[i] - F * INTERVAL * 0.001f * normalize(ball_velocity[i]);
			if (new_velocity[0] * ball_velocity[i][0] > 0 || new_velocity[2] * ball_velocity[i][2] > 0) {
				ball_velocity[i] = new_velocity;
			}
			else {
				ball_velocity[i] = vec3(0.0f);
			}
		}
		else {
			if (length(ball_velocity[i]) < 0.04f) {
				ball_velocity[i] = vec3(0.0f);
			}
		}
	}
}

void updateLoop() {
	auto dt = std::chrono::milliseconds(INTERVAL);
	state = 0;
	num_player = 0;

	// each frame
	while (true) {
		// game start, wait for 2 player to join to start
		if (state == 0) {
			initialize();
			if (num_player == 2) {
				state = 1;
			}
		}
		// wait for cue hit
		else if (state == 1) {
			// hit the cue go to state 2: ball moving
			if (check_cue_hit()) {
				state = 2;
			}
			// not hit yet, stay at current state
			else {
				state = 1;
			}
		}
		// let the ball moving
		else if (state == 2) {
			// all balls stop
			if (check_all_stop()) {
				if (from3) {
					state = 1;
					from3 = false;
					foul = false;
					cout << "go to state 1" << endl;
				}
				else {
					// make a foul, go to state 3: place cue ball and change the player
					if (!first_collision) {
						foul = true;
					}
					if (foul) {
						on_table[0] = false;
						cur_round = (cur_round + 1) % 2;
						foul = false;
						first_collision = false;
						pocketed = false;
						state = 3;
					}
					else if (!pocketed) {
						cur_round = (cur_round + 1) % 2;
						state = 1;
						foul = false;
						first_collision = false;
						pocketed = false;
					}
					else {
						state = 1;
						foul = false;
						first_collision = false;
						pocketed = false;
					}
				}
			}
			// not stop yet, stay ate current state
			else {
				state = 2;
				check_ball_hit();
				check_pocketed();          // if pocketed black 8 will direct set the state to 4: game over
				update_pos();              // update position and velocity   
			}
		}
		// free cue ball
		else if (state == 3) {
			if (on_table[0]) {
				state = 2;
				from3 = true;
			}
			else {
				state = 3;
			}
		}
		else if (state == 4) {
			if ((cur_round == 0 && lose) || (cur_round == 1 && !lose)) {
				cout << "player 1 lost." << endl;
				cout << "player 2 won." << endl;
			}
			else {
				cout << "player 1 win." << endl;
				cout << "player 2 lost." << endl;
			}
			state = 0;
		}

		std::this_thread::sleep_for(dt);
	}
}

void startServer() {
	rpc::server srv(PORT);
	std::cout << "Listening to port: " << PORT << std::endl;

	srv.bind("getStatus", [](const int& id, ClientData & info)
	{
		PlayerData data;

		// new gamer join
		if (id == -1) {
			data.id = num_player;
			num_player++;
		}
		// return its own id for joined player
		else {
			data.id = id;
		}
		data.state = state;
		data.cur_round = cur_round;
		data.selected = selected;
		// update data
		headPoses[data.id] = info.headPose;
		handPoses[data.id][0] = info.controllerPose[0];
		handPoses[data.id][1] = info.controllerPose[1];

		int oppo = (data.id + 1) % 2;
		data.headPose = headPoses[oppo];
		data.controllerPose[0] = handPoses[oppo][0];
		data.controllerPose[1] = handPoses[oppo][1];

		return data;
	});
	
	srv.bind("getStatus2", [](const int& id, ClientData2 & info)
	{
		// update time
		clock_t current = clock();
		float interval = (current - time_record[id]) / ((float)CLOCKS_PER_SEC / 1000);
		cue_velocity[id] = 1000.0f * (vec3(info.cue_point) - cue_position[id]) / interval;
		time_record[id] = current;

		// cue
		cuePoses[id] = info.cuePose;
		cue_position_pre[id] = cue_position[id];
		cue_position[id] = info.cue_point;
		hit[id] = info.hit;
		hold[id] = info.hold;

		// set white ball
		if (info.put_cue && id == cur_round && state == 3) {
			// get the location of white ball
			mat4 transf = mat4(1.0f);
			transf[3] = vec4(0.0f, 0.42f, 0.0f, 1.0f);
			vec3 white_position = vec3(transf * handPoses[id][0] * vec4(0.0f, 0.0f, 0.0f, 1.0f));
			// check whether in the table
			if (white_position.x > -MID + RADIUS && white_position.x < MID - RADIUS
				&& white_position.z > -2 * MID + RADIUS && white_position.z < 2 * MID - RADIUS
				&& white_position.y > RADIUS) 
			{
				bool valid = true;
				for (int k = 1; k < NUMBALL; k++) {
					if (on_table[k] && length(vec2(white_position.x, white_position.z) - vec2(ball_pos[k].x, ball_pos[k].z)) <= RADIUS) {
						valid = false;
					}
				}
				if (valid) {
					ball_pos[0] = white_position;
					ball_velocity[0] = vec3(0.0f, -0.001f, 0.0f);
					on_table[0] = true;
				}
			}
		}

		PlayerData2 data;
		// volume
		if (hit_volume[id] > 0 && hit_volume[id] < VOLUME_OFFSET) {
			hit_volume[id] = VOLUME_OFFSET;
		}
		
		data.hit_volume = std::min(hit_volume[id], 3.0f);
		data.pocketed = is_pocketed[id];
		data.cue_hit = std::min(cue_hit_vel[id] * 0.8f, 3.0f);
		data.wall_hit = std::min(hit_wall[id], 3.0f);

		int oppo = (id + 1) % 2;
		data.cuePose = cuePoses[oppo];
		data.hold = hold[oppo];
		// reset sound
		is_pocketed[id] = false;
		cue_hit_vel[id] = 0;
		hit_volume[id] = 0;
		hit_wall[id] = 0;
		
		return data;
	});
	
	srv.bind("getLocation", []()
	{
		LocationBall data;
		// send ball location
		for (int i = 0; i < NUMBALL; i++) {
			data.ball_pos[i] = ball_pos[i];
		}
		return data;
	});

	srv.bind("getRotation", []()
	{
		RotationBall data;
		// send ball location
		for (int i = 0; i < NUMBALL; i++) {
			data.ball_rotation[i] = ball_rotation[i];
		}
		return data;
	});

	srv.bind("getOnTable", []()
	{
		OnTableBall data;
		// send ball location
		for (int i = 0; i < NUMBALL; i++) {
			data.on_table[i] = on_table[i];
		}
		return data;
	});

	srv.run();
}

int main()
{ 
	std::thread reqHandleThread(startServer);
	std::thread updateLoopThread(updateLoop);
	reqHandleThread.join();
	updateLoopThread.join();
	return 0;
}
