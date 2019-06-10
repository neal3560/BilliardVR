// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
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
#include "ClientData.h"

#include <ctime>


using std::string;
using namespace glm;
using namespace std;

#define PORT 3560
#define NUMBALL 11
#define MID 0.68
#define RADIUS 0.033
#define POCKET_RADIUS 0.06
#define INTERVAL 5
#define F 0.3
#define G 10

#define VOLUME_OFFSET 0.8

mat4 headPoses[2];
mat4 handPoses[2][2];

clock_t time_record[2];
vec3 cue_position[2];
vec3 cue_position_pre[2];
vec3 cue_velocity[2];
bool hit[2];

//sound effect
float hit_volume;
bool is_pocketed = false;
float cue_hit_vel = 0;

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
}

void update_pos(){
	for (int i = 0; i < NUMBALL; i++) {
		// change location
		ball_pos[i] += INTERVAL * 0.001f * ball_velocity[i]; 
		// change rotation
		float angle = INTERVAL / float(RADIUS) * 0.001f * length(ball_velocity[i]);
		if (angle > 0) {
			vec3 axis = normalize(cross(vec3(0.0f, 1.0f, 0.0f), ball_velocity[i]));
			ball_rotation[i] = angleAxis(angle, axis) * ball_rotation[i];
		}
		// change velocity
		if (ball_pos[i].y > RADIUS) {
			ball_velocity[i].y -= G * INTERVAL * 0.001f;
		}
		else {
			vec3 new_velocity = ball_velocity[i] - float(F) * INTERVAL * 0.001f * normalize(ball_velocity[i]);
			if (new_velocity[0] * ball_velocity[i][0] > 0 || new_velocity[2] * ball_velocity[i][2] > 0) {
				ball_velocity[i] = new_velocity;
			}
			else {
				ball_velocity[i] = vec3(0.0f);
			}
		}
		// check whether pocketed
		for (int j = 0; j < 6; j++) {
			float dist_sq = pow(ball_pos[i].x - pocket_location[j].x, 2) + pow(ball_pos[i].z - pocket_location[j].y, 2);
			if (dist_sq <= pow(POCKET_RADIUS, 2)) {
				on_table[i] = false;
				is_pocketed = true;
			}
		}
	}
}

void check_collide() {

	// ball collision
	vector<int> collision;

	// detect collision
	for (int i = 0; i < NUMBALL - 1; i++) {
		if (on_table[i]) {
			for (int j = i + 1; j < NUMBALL; j++) {
				if (on_table[j]) {
					float overlap = 2 * RADIUS - length(ball_pos[i] - ball_pos[j]);
					if (overlap >= 0) {
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
		hit_volume += length(velocity_A_1 - velocity_B_1);
	}

	// table and wall collision
	for (int i = 0; i < NUMBALL; i++) {
		// table
		if (ball_pos[i].y < RADIUS) {
			ball_pos[i].y = RADIUS;
			ball_velocity[i].y = -0.25f * ball_velocity[i].y;
			if (ball_velocity[i].y < 0.5f) {
				ball_velocity[i].y = 0.0f;
			}
		}

		// left side
		if (ball_pos[i][0] - RADIUS <= -MID) {
			ball_pos[i][0] = RADIUS - MID;
			ball_velocity[i][0] = -ball_velocity[i][0];
		}
		// right side
		else if (ball_pos[i][0] + RADIUS >= MID) {
			ball_pos[i][0] = -RADIUS + MID;
			ball_velocity[i][0] = -ball_velocity[i][0];
		}

		// front side
		if (ball_pos[i][2] - RADIUS <= -MID * 2) {
			ball_pos[i][2] = RADIUS - MID * 2;
			ball_velocity[i][2] = -ball_velocity[i][2];
		}
		// rear side
		else if (ball_pos[i][2] + RADIUS >= MID * 2) {
			ball_pos[i][2] = MID * 2 - RADIUS;
			ball_velocity[i][2] = -ball_velocity[i][2];
		}
	}

	// cue hit
	for (int i = 0; i < 2; i++) {
		if (length(ball_velocity[0]) == 0 && hit[i]) {
			
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
				cue_hit_vel = length(ball_velocity[0]);
			}
		}
	}
}

void startServer() {
	rpc::server srv(PORT);
	std::cout << "Listening to port: " << PORT << std::endl;

	srv.bind("getStatus", [](const int& id, ClientData & info)
	{
		// update time
		clock_t current = clock();
		float interval = (current - time_record[id]) / ((float)CLOCKS_PER_SEC/1000);
		time_record[id] = current;

		// update data
		headPoses[id] = info.headPose;
		handPoses[id][0] = info.controllerPose[0];
		handPoses[id][1] = info.controllerPose[1];
		// cue
		cue_velocity[id] = 1000.0f * (vec3(info.cue_point) - cue_position[id]) / interval;
		cue_position_pre[id] = cue_position[id];
		cue_position[id] = info.cue_point;
		hit[id] = info.hit;

		// return value
		PlayerData data;

		// volume
		if (hit_volume > 0 && hit_volume < VOLUME_OFFSET)
			hit_volume = VOLUME_OFFSET;
		data.hit_volume = std::min(hit_volume,float(3.0)); 
		data.pocketed = is_pocketed;
		data.cue_hit = std::min(float(cue_hit_vel * 0.6), float(3.0));
		//reset
		is_pocketed = false;
		cue_hit_vel = 0;
		hit_volume = 0;

		if (id == 0) {
			data.headPose = headPoses[1];
			data.controllerPose[0] = handPoses[1][0];
			data.controllerPose[1] = handPoses[1][1];
		}
		else {
			data.headPose = headPoses[0];
			data.controllerPose[0] = handPoses[0][0];
			data.controllerPose[1] = handPoses[0][1];
		}

		// reset
		if (info.hit && on_table[0] == 0) {
			initialize();
		}

		// for debug
		//if (info.hit) {
		//	ball_velocity[0] = vec3(0, 0, -2);
		//}

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

void updateLoop() {
	auto dt = std::chrono::milliseconds(INTERVAL);
	initialize();
	
	// each frame
	while (true) {
		
		check_collide();

		update_pos();
			
		std::this_thread::sleep_for(dt);
	
	}
}

int main()
{ 
	std::thread reqHandleThread(startServer);
	std::thread updateLoopThread(updateLoop);
	reqHandleThread.join();
	updateLoopThread.join();
	return 0;
}
