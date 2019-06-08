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
#include "ClientData.h"



#define NUMLIGHT 3
#define NUMBALL 11

class Scene {

public:
	Scene();
	void render(const mat4& projection, const mat4& view, const mat4 controllerL, const mat4 controllerR, const mat4 rotationR, const PlayerData &playerData, 
		vec3 * ball_pos, quat * ball_rot, bool * ball_on, mat4 & cue_point, bool right_hold, bool left_hand, vec3 player_translate, float rotate_change);

	GLuint shader;

	mat4 transf;

	//models
	std::unique_ptr<Skybox> skybox;

	Model * hand;
	Model * head;
	
	Model * cue;
	Model * balls[11];
	Model * fabric;
	Model * base;

	Cube * floor;


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

	//light
	const float lightposn[NUMLIGHT * 4] = {
		0.0f, 2.0f, 0.0f, 1.0f,
		0.0f, 2.0f, 0.71f, 1.0f,
		0.0f, 2.0f, -0.71f, 1.0f,
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

	vec3 head_pos;
	quat head_rot;
	vec3 hand_pos[2];
	quat hand_rot[2];

	PlayerData playerData;
	vec3 ball_pos[NUMBALL];
	quat ball_rot[NUMBALL];
	bool ball_on[NUMBALL];

	bool right_hand;
	bool right_index;
	bool left_hand;

	// player movement
	vec3 player_translation;
	float player_rotation;

	// cue point
	mat4 cue_point;
};
#endif
