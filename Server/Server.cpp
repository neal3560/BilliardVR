// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "rpc/server.h"
#include <string>
#include <iostream>

// Shared struct
#include "Send.h"
#include "Receive.h"


using std::string;
using namespace glm;
using namespace std;

#define PORT 3560
#define NUMBALL 11
#define RADIUS 0.025
#define INTERVAL 10

Receive receive_data;
vec3 headPoses[2];
vec3 ball_pos[NUMBALL] = 
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
vec3 ball_velocity[NUMBALL] = {
	vec3(0.0f, 0.0f, 0.5f),
	vec3(0.0f),
	vec3(0.0f),
	vec3(0.0f),
	vec3(0.0f),
	vec3(0.0f),
	vec3(0.0f),
	vec3(0.0f),
	vec3(0.0f),
	vec3(0.0f),
	vec3(0.0f),
};

vec3 ball_velocity_next[NUMBALL];

const float f = 0.0f;

void update_pos(){
	for (int i = 0; i < NUMBALL; i++) {
		ball_pos[i] += INTERVAL * 0.001f * ball_velocity[i];
		vec3 new_velocity = ball_velocity[i] - f * INTERVAL * 0.001f * normalize(ball_velocity[i]);
		if (new_velocity[0] * ball_velocity[i][0] > 0 || new_velocity[2] * ball_velocity[i][2] > 0) {
			ball_velocity[i] = new_velocity;
		}
		else {
			ball_velocity[i] = vec3(0.0f);
		}
	}
}

void check_collide() {
	for (int i = 0; i < NUMBALL; i++) {
		ball_velocity_next[i] = ball_velocity[i];
	}

	for (int i = 0; i < NUMBALL; i++) {
		// ball collision
		for (int j = 0; j < NUMBALL; j++) {
			if (i != j && length(ball_pos[i] - ball_pos[j]) <= 2 * RADIUS) {
				if (i == 0) {
					cout << "collision" << endl;
				}
				// collide direction
				vec3 collide_direction = normalize(ball_pos[i] - ball_pos[j]);
				// decompose ball velocity along collide direction
				vec3 velocity_A_1 = dot(ball_velocity_next[i], collide_direction) * collide_direction;
				vec3 velocity_A_2 = ball_velocity_next[i] - velocity_A_1;
				vec3 velocity_B_1 = dot(ball_velocity[j], collide_direction) * collide_direction;
				// exchange velocity along collide direction
				ball_velocity_next[i] = velocity_B_1 + velocity_A_2;
			}
		}

		// left side
		if (ball_pos[i][0] - RADIUS <= -0.65f) {
			ball_velocity_next[i][0] = -ball_velocity_next[i][0];
		}
		// right side
		else if (ball_pos[i][0] + RADIUS >= 0.65f) {
			ball_velocity_next[i][0] = -ball_velocity_next[i][0];
		}

		// front side
		if (ball_pos[i][2] - RADIUS <= -1.3f) {
			ball_velocity_next[i][2] = -ball_velocity_next[i][2];
		}
		// rear side
		else if (ball_pos[i][2] + RADIUS >= 1.3f) {
			ball_velocity_next[i][2] = -ball_velocity_next[i][2];
		}
	}
	float energy = 0.0f;
	for (int i = 0; i < NUMBALL; i++) {
		ball_velocity[i] = ball_velocity_next[i];
		energy += length(ball_velocity[i]) * length(ball_velocity[i]);
	}
	cout << energy << endl;
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
		}
		else {
			receive_data.head_pos = headPoses[0];
		}

		// send ball location
		for (int i = 0; i < NUMBALL; i++) {
			receive_data.ball_pos[i] = ball_pos[i];
		}
			
		return receive_data;
	});

	srv.run();
}

void updateLoop() {
	auto dt = std::chrono::milliseconds(INTERVAL);
	
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
