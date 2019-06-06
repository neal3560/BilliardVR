// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "rpc/server.h"
#include <string>
#include <iostream>
#include <vector>
#include <math.h>

// Shared struct
#include "Send.h"
#include "Receive.h"


using std::string;
using namespace glm;
using namespace std;

#define PORT 3560
#define NUMBALL 11
#define RADIUS 0.025
#define POCKET_RADIUS 0.04
#define INTERVAL 1

Receive receive_data;
vec3 headPoses[2];

const vec3 ball_pos_init[NUMBALL] = 
{ 
	vec3(0.0f, RADIUS, 0.75f),      // cue ball
	vec3(0.0f, RADIUS, -0.75f),		// 1
	vec3(0.025f, RADIUS, -0.794f),   // 2
	vec3(-0.025f, RADIUS, -0.794f),		
	vec3(0.0f, RADIUS, -0.838f),		// 3
	vec3(0.05f, RADIUS, -0.838f),
	vec3(-0.05f, RADIUS, -0.838f),
	vec3(0.025f, RADIUS, -0.882f),	// 4
	vec3(-0.025f, RADIUS, -0.882f),
	vec3(0.075f, RADIUS, -0.882f),
	vec3(-0.075f, RADIUS, -0.882f),
};

vec2 pocket_location[6] = 
{
	vec2(-0.65f, -1.3f),
	vec2(0.65f, -1.3f),
	vec2(-0.65f, 0.0f),
	vec2(0.65f, 0.0f),
	vec2(-0.65f, 1.3f),
	vec2(0.65f, 1.3f)
};

vec3 ball_pos[NUMBALL];
vec3 ball_velocity[NUMBALL];
bool on_table[NUMBALL];

const float f = 0.0f;
float energy = 0.0f;



void initialize() {
	for (int i = 0; i < NUMBALL; i++) {
		ball_pos[i] = ball_pos_init[i];
		ball_velocity[i] = vec3(0.0f);
		on_table[i] = true;
	}
}

void update_pos(){
	for (int i = 0; i < NUMBALL; i++) {
		// change location
		ball_pos[i] += INTERVAL * 0.001f * ball_velocity[i];
		vec3 new_velocity = ball_velocity[i] - f * INTERVAL * 0.001f * normalize(ball_velocity[i]);
		if (new_velocity[0] * ball_velocity[i][0] > 0 || new_velocity[2] * ball_velocity[i][2] > 0) {
			ball_velocity[i] = new_velocity;
		}
		else {
			ball_velocity[i] = vec3(0.0f);
		}
		// check whether pocketed
		for (int j = 0; j < 6; j++) {
			float dist_sq = pow(ball_pos[i].x - pocket_location[j].x, 2) + pow(ball_pos[i].z - pocket_location[j].y, 2);
			if (dist_sq <= pow(POCKET_RADIUS, 2)) {
				on_table[i] = false;
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
	}

	// wall collision
	for (int i = 0; i < NUMBALL; i++) {
		// left side
		if (ball_pos[i][0] - RADIUS <= -0.65f) {
			ball_pos[i][0] = RADIUS - 0.65f;
			ball_velocity[i][0] = -ball_velocity[i][0];
		}
		// right side
		else if (ball_pos[i][0] + RADIUS >= 0.65f) {
			ball_pos[i][0] = -RADIUS + 0.65f;
			ball_velocity[i][0] = -ball_velocity[i][0];
		}

		// front side
		if (ball_pos[i][2] - RADIUS <= -1.3f) {
			ball_pos[i][2] = RADIUS - 1.3f;
			ball_velocity[i][2] = -ball_velocity[i][2];
		}
		// rear side
		else if (ball_pos[i][2] + RADIUS >= 1.3f) {
			ball_pos[i][2] = 1.3f - RADIUS;
			ball_velocity[i][2] = -ball_velocity[i][2];
		}
	}
		

	float new_energy = 0.0f;
	for (int i = 0; i < NUMBALL; i++) {
		new_energy += length(ball_velocity[i]) * length(ball_velocity[i]);
	}
	if (new_energy != energy) {
		cout << new_energy << endl;
	}
	energy = new_energy;
}



void startServer() {
	rpc::server srv(PORT);
	std::cout << "Listening to port: " << PORT << std::endl;

	srv.bind("echo", [](const int& id, Send & info)
	{
		headPoses[id] = info.head_pos;
		
		// return value : that will be returned back to client side
		if (id == 0) {
			receive_data.head_pos = headPoses[1];
			if (info.hit) {
				vec3 direction = ball_pos[0] - info.controller_location + vec3(0.0f, 0.0f, -1.3f);
				direction = normalize(vec3(direction.x, 0.0f, direction.z));
				ball_velocity[0] = 2.0f * direction;
			}
		}
		else {
			receive_data.head_pos = headPoses[0];
		}

		// send ball location
		for (int i = 0; i < NUMBALL; i++) {
			receive_data.ball_pos[i] = ball_pos[i];
			receive_data.on_table[i] = on_table[i];
		}
			
		return receive_data;
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
