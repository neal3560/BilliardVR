
#include "Scene.h"
using namespace std;
int player_id = 0;

#define CUELENGTH 1.2
#define MOVESTEP 0.01
#define ROTATESTEP 0.006

  ExampleApp::ExampleApp()
  {
	  client = new rpc::client("localhost", 3560);
	  std::cout << "Connected" << std::endl;

	  right_hand = false;
	  right_index = false;
	  left_hand = false;

	  player_translation = vec3(0.0f, 0.0f, 0.0f);
	  player_rotation = 0.0f;
  }


  void ExampleApp::initGl()
  {
    RiftApp::initGl();
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    ovr_RecenterTrackingOrigin(_session);
    scene = std::shared_ptr<Scene>(new Scene());

  }

  void ExampleApp::shutdownGl()
  {
    scene.reset();
  }

  void ExampleApp::updateState()
  {	  
	  // ==================== button state ========================
	  ovrInputState inputState;
	  ovr_GetInputState(_session, ovrControllerType_Touch, &inputState);

	  // A button
	  bool A = inputState.Buttons & ovrButton_A;

	  // right hand trigger
	  right_hand = inputState.HandTrigger[ovrHand_Right] > 0.5f;
	  right_index = inputState.IndexTrigger[ovrHand_Right] > 0.5f;
	  // left hand
	  left_hand = inputState.HandTrigger[ovrHand_Left] > 0.5f;

	  /********************** player movement ***************************/
	  // move based on user's view
	  vec2 LT_stick = ovr::toGlm(inputState.Thumbstick[0]);
	  if (length(LT_stick) < 0.1f) {
		  LT_stick = vec2(0.0f, 0.0f);
	  }
	  vec3 move = mat4_cast(head_rot) * rotate(mat4(1.0f), player_rotation, vec3(0, 1, 0)) * vec4(LT_stick[0], 0, -LT_stick[1], 0);
	  player_translation.x += move.x * MOVESTEP;
	  player_translation.z += move.z * MOVESTEP;
	  // up
	  if (inputState.Buttons & ovrButton_Y) {
		  player_translation.y += MOVESTEP;
	  }
	  // down
	  if (inputState.Buttons & ovrButton_X) {
		  player_translation.y -= MOVESTEP;
	  }
	  /********************** player rotatation **************************/
	  // RT stick
	  vec2 RT_stick = ovr::toGlm(inputState.Thumbstick[1]);
	  if (RT_stick[0] > 0.3f) {
		  player_rotation -= ROTATESTEP;
	  }
	  if (RT_stick[0] < -0.3f) {
		  player_rotation += ROTATESTEP;
	  }

	  /* ================= controller/head tracking =================== */
	  // head
	  head_pos = (ovr::toGlm(eyePoses[0].Position) + ovr::toGlm(eyePoses[1].Position)) / 2.0f;
	  head_rot = ovr::toGlm(eyePoses[0].Orientation);

	  double ftiming = ovr_GetPredictedDisplayTime(_session, 0);
	  ovrTrackingState hmdState = ovr_GetTrackingState(_session, ftiming, ovrTrue);
	  // left controller
	  ovrPoseStatef leftHandPoseState = hmdState.HandPoses[ovrHand_Left];
	  hand_pos[0] = ovr::toGlm(leftHandPoseState.ThePose.Position);
	  hand_rot[0] = ovr::toGlm(ovrQuatf(leftHandPoseState.ThePose.Orientation));
	  // right controller
	  ovrPoseStatef rightHandPoseState = hmdState.HandPoses[ovrHand_Right];
	  hand_pos[1] = ovr::toGlm(rightHandPoseState.ThePose.Position);
	  hand_rot[1] = ovr::toGlm(ovrQuatf(rightHandPoseState.ThePose.Orientation));

	  /* ============ connect server ===================*/

	  // upload
	  ClientData data;  
	  data.head_pos = head_pos;
	  data.head_rot = head_rot;
	  data.player_translation = player_translation;
	  data.player_rotation = player_rotation;
	  for (int i = 0; i < 2; i++) {
		  data.controller_location[i] = hand_pos[i];
		  data.controller_rotation[i] = hand_rot[i];
	  }
	  data.hit = right_hand && right_index && left_hand;
	  data.cue_point = translate(mat4(1.0f), vec3(0.0f, 0.6f, 0.0f)) * cue_point * vec4(0.0f,0.0f,0.0f,1.0f);
	  // download
	  playerData = client->call("getStatus", player_id, data).as<PlayerData>();
	  LocationBall pos_data = client->call("getLocation").as<LocationBall>();
	  RotationBall rot_data = client->call("getRotation").as<RotationBall>();
	  OnTableBall on_data = client->call("getOnTable").as<OnTableBall>();
	  for (int i = 0; i < NUMBALL; i++) {
		  ball_pos[i] = pos_data.ball_pos[i];
		  ball_rot[i] = rot_data.ball_rotation[i];
		  ball_on[i] = on_data.on_table[i];
	  }
  }

  void ExampleApp::renderScene(const glm::mat4& projection, const ovrPosef & eyePose)
  {
	  mat4 bodyRotation = rotate(mat4(1.0f), player_rotation, vec3(0, 1, 0));
	  mat4 bodyTranslation = translate(mat4(1.0f), player_translation);
	  head_pos = mat3(bodyRotation) * head_pos;
	  mat4 transf = bodyTranslation * translate(mat4(1.0f), head_pos) * bodyRotation * inverse(translate(mat4(1.0f), head_pos));
	  // controller matrix
	  mat4 left_controller = transf * translate(mat4(1.0f), hand_pos[0]) * mat4_cast(hand_rot[0]);
	  mat4 right_controller = transf * translate(mat4(1.0f), hand_pos[1]) * mat4_cast(hand_rot[1]);
	  mat4 right_rotation = mat4_cast(hand_rot[1]); 
	  // modelview matrix
	  mat4 modelview = transf * ovr::toGlm(eyePose);
	  // call render
	  scene->render(projection, inverse(modelview),
		  left_controller, right_controller, right_rotation,
		  playerData, 
		  ball_pos, ball_rot, ball_on, cue_point,
		  right_hand && right_index, left_hand,
		  player_translation, player_rotation
	  );
  } 

 

Scene::Scene()
{
	// Shader Program 
	shader = LoadShaders("shader.vert", "shader.frag");
	
	// player model
	hand = new Model("models/hand.obj", false);
	head = new Model("models/sphere.obj", false);
	cue = new Model("models/Cue2.obj", false);

	// table model
	fabric = new Model("models/fabric.obj", false);
	base = new Model("models/base.obj", false);

	// ball model
	for (int i = 0; i < 11; i++) {
		balls[i] = new Model("models/Ball_" + to_string(i) + ".obj", false);
		balls[i]->mode = 1;
	}

	transf = translate(mat4(1.0f), vec3(0.0f, -0.6f, 0.0f));

	hand->mode = 3;
	cue->mode = 1;

	fabric->mode = 1;
	fabric->toWorld = translate(mat4(1.0f), vec3(0.0f, -0.925f, 0.0f)) * transf * scale(mat4(1.0f), vec3(0.012f));

	base->mode = 1;
	base->toWorld = translate(mat4(1.0f), vec3(0.0f, -0.925f, 0.0f)) * transf * scale(mat4(1.0f), vec3(0.012f));

	// floor 
	floor = new Cube();

	// 10m wide sky box: size doesn't matter though
	skybox = std::make_unique<Skybox>("skybox");
	skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));

	// load texture
	diffuse_fabric = TextureFromFile("Fabric_Diffuse.png", "texture");
	specular_fabric = TextureFromFile("Fabric_Glossiness.png", "texture");
	diffuse_base = TextureFromFile("Table_Diffuse.png", "texture");
	specular_base = TextureFromFile("Table_Glossiness.png", "texture");
	diffuse_ball = TextureFromFile("Balls_Diffuse.png", "texture");
	specular_ball = TextureFromFile("Balls_Glossiness.png", "texture");
	diffuse_cue = TextureFromFile("Cues_Diffuse.png", "texture");
	specular_cue = TextureFromFile("Cues_Glossiness.png", "texture");
	diffuse_floor = TextureFromFile("floor.png", "texture");
}

void Scene::render(
	const mat4& projection, const mat4& view, 
	const mat4 controllerL, const mat4 controllerR, const mat4 rotationR,  
	const PlayerData & playerData, 
	vec3 * ball_pos, quat * ball_rot, bool * ball_on, mat4& cue_point, 
	bool rightHold, bool left_hand, 
	vec3 player_translate, float rotate_change)
{	
	//lights
	float lightransf[4 * NUMLIGHT];
	for (int i = 0; i < NUMLIGHT; i++) {
		vec4 lightposAfter = view * transf * vec4(lightposn[4 * i], lightposn[4 * i + 1], lightposn[4 * i + 2], lightposn[4 * i + 3]);
		lightransf[4 * i] = lightposAfter[0];
		lightransf[4 * i + 1] = lightposAfter[1];
		lightransf[4 * i + 2] = lightposAfter[2];
		lightransf[4 * i + 3] = lightposAfter[3];
	}
	glUniform1i(glGetUniformLocation(shader, "numused"), NUMLIGHT);
	glUniform4fv(glGetUniformLocation(shader, "lightcolor"), 1, lightcolor);
	glUniform4fv(glGetUniformLocation(shader, "lightposn"), 1, lightransf);

	//skybox
	skybox->draw(shader, projection, view);

	//balls
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, diffuse_ball);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, specular_ball);
	glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 1);
	glUniform1i(glGetUniformLocation(shader, "texture_specular"), 2);
	for (int i = 0; i < NUMBALL; i++) {
		balls[i]->toWorld =  transf 
			* translate(mat4(1.0f), ball_pos[i]) 
			* toMat4(ball_rot[i])
			* scale(mat4(1.0f), vec3(0.012f));
		if (ball_on[i]) {
			balls[i]->Draw(shader, projection, view);
		}
	}
	//cues
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, diffuse_cue);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, specular_cue);
	glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 1);
	glUniform1i(glGetUniformLocation(shader, "texture_specular"), 2);

	vec3 hands = vec3(controllerL[3]) - vec3(controllerR[3]);
	vec3 axis = normalize(cross(hands, vec3(0.0f, -1.0f, 0.0f)));
	float angle = acos(dot(normalize(hands), vec3(0.0f, 1.0f, 0.0f)));
	mat4 rotation = mat4_cast(angleAxis(angle, axis));

	cue_point = translate(mat4(1.0f), vec3(controllerR[3])) * rotation * translate(mat4(1.0f), vec3(0.0f, CUELENGTH, 0.0f));

	if (rightHold) {
		
		// connect to left hand when triggered.
		if (left_hand) {
			// cue point----------------------
			head->toWorld = cue_point * scale(mat4(1.0f), vec3(0.003f));
			head->Draw(shader, projection, view);
		}
		else {
			rotation = rotationR;
		}

		cue->toWorld = translate(mat4(1.0f), vec3(controllerR[3])) * rotation * scale(mat4(1.0f), vec3(0.008f));
		cue->Draw(shader, projection, view);
	}

	// render the controller
	mat4 hand_transf = glm::rotate(mat4(1.0f), 3.14f * 0.5f, vec3(0, 1, 0)) * glm::rotate(mat4(1.0f), 3.14f, vec3(0, 0, 1)) * scale(mat4(1.0f), vec3(0.0022f));
	mat4 hand_reflection = mat4(-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	hand->toWorld = controllerL * hand_reflection* hand_transf;
	hand->Draw(shader, projection, view);
	hand->toWorld = controllerR * hand_transf;
	hand->Draw(shader, projection, view);

	//opponent's head and controller
	/*
	head->toWorld = translate(mat4(1.0f), playerData.head_pos) * scale(mat4(1.0f), vec3(0.04f));
	head->Draw(shader, projection, view);
	hand->toWorld = playerData.controller_location[0] * playerData.controller_rotation[0] * hand_reflection * hand_transf;
	hand->Draw(shader, projection, view);
	hand->toWorld = playerData.controller_location[1] * playerData.controller_rotation[1] * hand_transf;
	hand->Draw(shader, projection, view);
	*/

	// table fabric
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, diffuse_fabric);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, specular_fabric);
	glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 1);
	glUniform1i(glGetUniformLocation(shader, "texture_specular"), 2);
	fabric->Draw(shader, projection, view);

	// floor
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, diffuse_floor);
	glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 1);
	floor->mode = 5;
	floor->toWorld = translate(mat4(1.0f), vec3(0.0f, -1.52f, 0.0f)) * scale(mat4(1.0f), vec3(8.0f, 0.00002f, 8.0f));
	floor->draw(shader, projection, view);

	// table base
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, diffuse_base);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, specular_base);
	glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 1);
	glUniform1i(glGetUniformLocation(shader, "texture_specular"), 2);
	base->Draw(shader, projection, view);

}

int main()
{
	int result = -1;
	if (!OVR_SUCCESS(ovr_Initialize(nullptr)))
	{
		FAIL("Failed to initialize the Oculus SDK");
	}
	result = ExampleApp().run();
	ovr_Shutdown();
	return result;
}


