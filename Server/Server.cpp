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

#define PORT 3560

Receive data;
vec3 headPoses[2];

void startServer() {
	rpc::server srv(PORT);
	std::cout << "Listening to port: " << PORT << std::endl;

	srv.bind("echo", [](const int& id, Send & info)
	{
		headPoses[id] = info.head_pos;
		//std::cout << "Position: " << "  "<<info.head_pos[0] <<"  "<< info.head_pos[1]<<"  "<< info.head_pos[2]<<std::endl;
		// return value : that will be returned back to client side
		if (id == 0) {
			data.head_pos = headPoses[1];
		}
		else {
			data.head_pos = headPoses[0];
		}
			
		return data;
	});

	srv.run();
}

void updateLoop() {
	auto dt = std::chrono::milliseconds(30);
	while (true) {
		// ... do data update
		// sleep at the end
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
