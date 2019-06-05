#ifndef SCENE_H
#define SCENE_H
#include "Tool.h"
#include "pch.h"
#include <iostream>

#include "rpc/client.h"
#include <string>

#include "Send.h"
#include "Receive.h"

class Scene {

public:
	Scene();
	void render(const mat4& projection, const mat4& view, const mat4 controller, const Receive & info);

	GLuint shader;

	std::unique_ptr<Skybox> skybox;
	Model * hand;
	std::unique_ptr<Cube> table;

	Model * head;

};

class ExampleApp : public RiftApp {
public:
	ExampleApp();

protected:
	void initGl() override;
	void shutdownGl() override;
	void updateState() override;
	void renderScene(const glm::mat4& projection, const glm::mat4& headPose) override;

private:
	rpc::client *client;
	std::shared_ptr<Scene> scene;
	Receive info;

	vec3 rightHandPosition;
	quat rightHandRotation;

	bool LT_pre;
	bool RT_pre;

	bool hand_mode;
	bool freeze_mode;
	bool debug_mode;

	float cube_size;
	vec3 cube_translation;
};
#endif
