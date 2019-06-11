#ifndef SCENE_H
#define SCENE_H
#include "Tool.h"
#include "pch.h"
#include <iostream>

#include "rpc/client.h"
#include <string>

#include "LocationBall.h"
#include "RotationBall.h"
#include "OnTableBall.h"
#include "PlayerData.h"
#include "PlayerData2.h"
#include "ClientData.h"
#include "ClientData2.h"



#define NUMLIGHT 3
#define NUMBALL 16

class Scene {

public:
	Scene(mat4 transf);
	void render(const mat4& projection, const mat4& view, 
				const mat4 controllerL, const mat4 controllerR,  const mat4 cue, 
				const PlayerData &playerData, const PlayerData2 & playerData2, const int & ID,
				vec3 * ball_pos, quat * ball_rot, bool * ball_on, 
				bool hold_cue, float targetRotation);

	GLuint shader;

	mat4 transf;

	//models
	std::unique_ptr<Skybox> skybox;

	Model * hand;
	Model * head;

	Model * cue_head;
	
	Model * cue;
	Model * balls[NUMBALL];
	Model * fabric;
	Model * base;

	Cube * floor;

	Cube * face;


	//texture
	int diffuse_fabric;
	int specular_fabric;

	int diffuse_base;
	int specular_base;

	int diffuse_ball;
	int specular_ball;

	int diffuse_cue;
	int specular_cue;

	int diffuse_floor;

	int head_face;
	int head_side;

	//light
	const float lightposn[NUMLIGHT * 4] = {
		0.0f, 1.5f, 0.0f, 1.0f,
		0.0f, 1.5f, 0.71f, 1.0f,
		0.0f, 1.5f, -0.71f, 1.0f,
	};
	const float lightcolor[NUMLIGHT * 4] = {
		0.7f, 0.7f, 0.7f, 1.0f,
		0.7f, 0.7f, 0.7f, 1.0f,
		0.7f, 0.7f, 0.7f, 1.0f,
	};

};

class ExampleApp : public RiftApp {
public:
	ExampleApp();

protected:
	void initGl() override;
	void shutdownGl() override;
	void updateState() override;
	void renderScene(const glm::mat4& projection, const ovrPosef & eyePose) override;



private:
	rpc::client *client;
	std::shared_ptr<Scene> scene;
	mat4 virtual_transf;   //real to virtual coordinates transformation
	vec3 world_origin;     //world origin coordinates in virtual world

	mat4 transf;

	vec3 head_pos;         //head position related to real world coordinates
	quat head_rot;	       //head rotation related to real world coordinates
	
	mat4 hand_pose[2];
	mat4 cue_pose;

	int state;
	int player_id;

	PlayerData playerData;
	PlayerData2 playerData2;
	vec3 ball_pos[NUMBALL];
	quat ball_rot[NUMBALL];
	bool ball_on[NUMBALL];

	bool right_hold;
	bool left_hand;

	bool left_index_pre;
	bool left_index;

	// player movement
	vec3 player_translation;
	float player_rotation;

	// cue point
	mat4 cue_point;

	//auto rotate
	float targetRotation;
};
#endif
