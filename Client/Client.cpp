
#include "Scene.h"
#include <irrklang/irrKlang.h>

using namespace std;
using namespace irrklang;

#define PLAYERID 0	 // OSCAR 1; NEAL 0

#define CUELENGTH 1.5f
#define HOLDPOINT 0.33f
#define MOVESTEP 0.008f
#define ROTATESTEP 0.008f


// sound engine. LEAVE IT HERE
ISoundEngine * ball_engine = createIrrKlangDevice();
ISoundEngine * pocket_engine = createIrrKlangDevice();
ISoundEngine * cue_engine = createIrrKlangDevice();
ISoundEngine * bgm_engine = createIrrKlangDevice();
ISoundEngine * wall_engine = createIrrKlangDevice();


  ExampleApp::ExampleApp()
  {
	  client = new rpc::client("128.54.70.74", 3560);
	  std::cout << "Connected" << std::endl;

	  right_hold = false;
	  left_hand = false;
	  
	  transf = translate(mat4(1.0f), vec3(0.0f, -0.42f, 0.0f));

	  player_translation = vec3(0.0f, 0.0f, 1.6f);
	  player_rotation = 0.0f;
	  world_origin = vec3(0.0f);

	  left_index = false;
	  left_index_pre = false;

	  state = 0;
	  player_id = -1;

  }


  void ExampleApp::initGl()
  {
    RiftApp::initGl();
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    ovr_RecenterTrackingOrigin(_session);
    scene = std::shared_ptr<Scene>(new Scene(transf));

	waveOutSetVolume(NULL, 0xFFFFFFFF);
	// bmg sound
	ISound * sound_bgm = bgm_engine->play2D("sound/breakout.mp3", true,true,true); // initly paused
	sound_bgm->setVolume(0.07f);
	sound_bgm->setIsPaused(false);
	
  }

  void ExampleApp::shutdownGl()
  {
    scene.reset();
  }

  void ExampleApp::updateState()
  {	  

	  /* ================= controller/head tracking =================== */
	  // head
	  head_pos = (ovr::toGlm(eyePoses[0].Position) + ovr::toGlm(eyePoses[1].Position)) / 2.0f;
	  head_rot = ovr::toGlm(eyePoses[0].Orientation);
	  // controller
	  double ftiming = ovr_GetPredictedDisplayTime(_session, 0);
	  ovrTrackingState hmdState = ovr_GetTrackingState(_session, ftiming, ovrTrue);
	  // left
	  ovrPoseStatef leftHandPoseState = hmdState.HandPoses[ovrHand_Left];
	  hand_pose[0] = ovr::toGlm(leftHandPoseState.ThePose);
	  // right
	  ovrPoseStatef rightHandPoseState = hmdState.HandPoses[ovrHand_Right];
	  hand_pose[1] = ovr::toGlm(rightHandPoseState.ThePose);

	  // ==================== button state ========================
	  ovrInputState inputState;
	  ovr_GetInputState(_session, ovrControllerType_Touch, &inputState);

	  // left index trigger
	 
	  bool new_index = inputState.IndexTrigger[ovrHand_Left] > 0.5f;
	  if (left_index_pre == 0 && new_index == 1){
		  left_index = true;
	  }
	  else {
		  left_index = 0;
	  }
	  left_index_pre = new_index;

	  // right hand trigger
	  right_hold = inputState.HandTrigger[ovrHand_Right] > 0.5f && inputState.IndexTrigger[ovrHand_Right] > 0.5f;
	 
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
	  mat4 bodyTranslation = translate(mat4(1.0f), player_translation);
	  /********************** player rotatation **************************/
	  // RT stick
	  vec2 RT_stick = ovr::toGlm(inputState.Thumbstick[1]);
	  mat4 bodyRotation = mat4(1.0f);
	  if (RT_stick[0] > 0.3f) {
		  player_rotation -= ROTATESTEP;
		  bodyRotation = rotate(mat4(1.0f), float(-ROTATESTEP), vec3(0, 1, 0));
	  }
	  if (RT_stick[0] < -0.3f) {
		  player_rotation += ROTATESTEP;
		  bodyRotation = rotate(mat4(1.0f), float(ROTATESTEP), vec3(0, 1, 0));
	  }
	  vec3 virtual_head_pos = translate(mat4(1.0f), world_origin) * rotate(mat4(1.0f), player_rotation, vec3(0, 1, 0)) * vec4(head_pos, 1.0f);
	  mat4 origin_transf = translate(mat4(1.0f), virtual_head_pos) * bodyRotation * translate(mat4(1.0f), -virtual_head_pos);
	  world_origin = vec3(origin_transf * vec4(world_origin, 1.0f));
	  virtual_transf = bodyTranslation * translate(mat4(1.0f), world_origin) * rotate(mat4(1.0f), player_rotation, vec3(0, 1, 0));

	  /******************** update position *****************************/
	  // controller matrix
	  hand_pose[0] = virtual_transf * hand_pose[0];
	  hand_pose[1] = virtual_transf * hand_pose[1];

	  // cue position
	  vec3 hands = vec3(hand_pose[0][3]) - vec3(hand_pose[1][3]);
	  vec3 axis = normalize(cross(hands, vec3(0.0f, -1.0f, 0.0f)));
	  float angle = acos(dot(normalize(hands), vec3(0.0f, 1.0f, 0.0f)));
	  mat4 rotation = mat4_cast(angleAxis(angle, axis));
	  
	  cue_pose = translate(mat4(1.0f), vec3(hand_pose[1][3])) * rotation;
	  vec4 cue_point = inverse(transf) * cue_pose * vec4(0.0f, CUELENGTH - HOLDPOINT, 0.0f, 1.0f);
	  if (inputState.HandTrigger[ovrHand_Left] <= 0.5f) {
		  cue_pose = hand_pose[1];
	  }

	  /* ============ connect server ===================*/
	  // upload
	  ClientData data;  
	  data.headPose = virtual_transf * translate(mat4(1.0f),head_pos) *  mat4_cast(head_rot);
	  data.controllerPose[0] = hand_pose[0];
	  data.controllerPose[1] = hand_pose[1];
	  
	  ClientData2 data2;
	  data2.put_cue = left_index;
	  data2.cuePose = cue_pose;
	  data2.hold = right_hold;
	  data2.hit = right_hold && left_hand;
	  data2.cue_point = inverse(transf) * cue_pose * vec4(0.0f, CUELENGTH - HOLDPOINT, 0.0f, 1.0f);

	  // download
	  // retrieve user id
	  playerData = client->call("getStatus", player_id, data).as<PlayerData>();
	  player_id = playerData.id;

	  // retreive other data
	  playerData2 = client->call("getStatus2", player_id, data2).as<PlayerData2>();
	  LocationBall pos_data = client->call("getLocation").as<LocationBall>();
	  RotationBall rot_data = client->call("getRotation").as<RotationBall>();
	  OnTableBall on_data = client->call("getOnTable").as<OnTableBall>();
	  for (int i = 0; i < NUMBALL; i++) {
		  ball_pos[i] = pos_data.ball_pos[i];
		  ball_rot[i] = rot_data.ball_rotation[i];
		  ball_on[i] = on_data.on_table[i];
	  }

	  /********************** Sound ****************************/
	  if (playerData2.wall_hit > 0) {
		  ISound * soundobj = wall_engine->play2D("sound/hit_wall.wav", false, true, true); // create a sound obj intially paused
		  soundobj->setVolume(playerData2.wall_hit / 3.0f * 0.2f); // volume between 0~1 (0.7 because the original wav fle is lound)
		  soundobj->setIsPaused(false);
	  }
	  // ball hit	
	  if (playerData2.hit_volume > 0) {
		  ISound * soundobj = ball_engine->play2D("sound/ball_hit.wav",false,true,true); // create a sound obj intially paused
		  soundobj->setVolume(playerData2.hit_volume / 3.0f * 0.7f); // volume between 0~1 (0.7 because the original wav fle is lound)
		  soundobj->setIsPaused(false);	
	  }
	  // pocket
	  if (playerData2.pocketed > 0) {
		  pocket_engine->stopAllSounds();
		  ISound * soundobj = pocket_engine->play2D("sound/pocket.wav", false, true, true); // create a sound obj intially paused
		  soundobj->setVolume(0.3f); // volume between 0~1
		  soundobj->setIsPaused(false);
	  }
	  else if (playerData2.pocketed < 0) {
		  pocket_engine->stopAllSounds();
		  ISound * soundobj = pocket_engine->play2D("sound/cue_pocket.wav", false, true, true); // create a sound obj intially paused
		  soundobj->setVolume(0.4f); // volume between 0~1
		  soundobj->setIsPaused(false);
	  }
	  // cue hit
	  if (playerData2.cue_hit > 0) {
		  ISound * soundobj = cue_engine->play2D("sound/cue.wav", false, true, true); // create a sound obj intially paused
		  soundobj->setVolume(playerData2.cue_hit/3.0f ); // volume between 0~1 
		  soundobj->setIsPaused(false);
	  }
  }

  void ExampleApp::renderScene(const glm::mat4& projection, const ovrPosef & eyePose)
  {
	  // modelview matrix
	  mat4 modelview = virtual_transf * ovr::toGlm(eyePose);
	  // call render
	  scene->render(projection, inverse(modelview),
		  hand_pose[0], hand_pose[1], cue_pose,
		  playerData, playerData2, player_id,
		  ball_pos, ball_rot, ball_on,
		  right_hold
	  );
  } 

 

Scene::Scene(mat4 transf)
{
	// Shader Program 
	shader = LoadShaders("shader.vert", "shader.frag");
	
	// player model
	hand = new Model("models/hand.obj", false);
	head = new Model("models/sphere.obj", false);
	cue = new Model("models/Cue2.obj", false);
	cue_head = new Model("models/sphere.obj", false);

	// table model
	fabric = new Model("models/fabric.obj", false);
	base = new Model("models/base.obj", false);

	// ball model
	for (int i = 0; i < NUMBALL; i++) {
		balls[i] = new Model("models/Ball_" + to_string(i) + ".obj", false);
		balls[i]->mode = 1;
	}
	this->transf = transf;

	hand->mode = 0;
	cue_head->mode = 0;
	cue->mode = 1;

	fabric->mode = 1;
	fabric->toWorld = translate(mat4(1.0f), vec3(0.0f, -0.926f, 0.0f)) * transf * scale(mat4(1.0f), vec3(0.012f));

	base->mode = 1;
	base->toWorld = translate(mat4(1.0f), vec3(0.0f, -0.926f, 0.0f)) * transf * scale(mat4(1.0f), vec3(0.012f));

	// floor 
	floor = new Cube();
	floor->mode = 3;
	floor->toWorld = transf * translate(mat4(1.0f), vec3(0.0f, -0.92f, 0.0f)) * scale(mat4(1.0f), vec3(5.0f, 0.00002f, 5.0f));

	face = new Cube();

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
	diffuse_floor = TextureFromFile("floor3.jpg", "texture");
	// opponent player's head -- 
	if (PLAYERID == 0) {
		// opponent is oscar
		head_face = TextureFromFile("front_1.png", "texture");
		head_side = TextureFromFile("side_1.png", "texture");
	}
	else {
		// opponent is neal
		head_face = TextureFromFile("front_2.jpg", "texture");
		head_side = TextureFromFile("side_2.png", "texture");
	}

}

void Scene::render(
	const mat4& projection, const mat4& view, 
	const mat4 controllerL, const mat4 controllerR, const mat4 cue_pose,
	const PlayerData & playerData, const PlayerData2 & playerData2, const int & ID,
	vec3 * ball_pos, quat * ball_rot, bool * ball_on,
	bool hold_cue)
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

	// render the controller
	mat4 hand_transf = glm::rotate(mat4(1.0f), 3.14f * 0.5f, vec3(0, 1, 0)) * glm::rotate(mat4(1.0f), 3.14f, vec3(0, 0, 1)) * scale(mat4(1.0f), vec3(0.00228f));
	mat4 hand_reflection = mat4(-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	hand->toWorld = controllerL * hand_reflection* hand_transf;
	hand->Draw(shader, projection, view);
	hand->toWorld = controllerR * hand_transf;
	hand->Draw(shader, projection, view);

	//cues
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, diffuse_cue);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, specular_cue);
	glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 1);
	glUniform1i(glGetUniformLocation(shader, "texture_specular"), 2);
	if (hold_cue) {
		cue->toWorld = cue_pose * translate(mat4(1.0f), vec3(0.0f, -HOLDPOINT, 0.0f)) * scale(mat4(1.0f), vec3(0.0101f));
		cue->Draw(shader, projection, view);
	}

	// render opponent when the other gamer join
	if (playerData.state > 0) {
		if (playerData2.hold) {
			cue->toWorld = playerData2.cuePose * translate(mat4(1.0f), vec3(0.0f, -HOLDPOINT, 0.0f)) * scale(mat4(1.0f), vec3(0.0101f));
			cue->Draw(shader, projection, view);
		}
		//opponent's head , controller and cue
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, head_face);
		glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 1);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, head_side);
		glUniform1i(glGetUniformLocation(shader, "texture_specular"), 2);
		glUniform1i(glGetUniformLocation(shader, "playerid"), (PLAYERID + 1) % 2);
		face->toWorld = playerData.headPose * rotate(mat4(1.0f), 3.14f, vec3(0, 1, 0)) * scale(mat4(1.0f), vec3(0.1f));
		face->mode = 5;
		face->draw(shader, projection, view);

		hand->toWorld = playerData.controllerPose[0] * hand_reflection * hand_transf;
		hand->Draw(shader, projection, view);
		hand->toWorld = playerData.controllerPose[1] * hand_transf;
		hand->Draw(shader, projection, view);
	}

	//balls
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, diffuse_ball);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, specular_ball);
	glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 1);
	glUniform1i(glGetUniformLocation(shader, "texture_specular"), 2);

	// cue ball
	if (playerData.state == 3) {
		if (playerData.cur_round == ID) {
			balls[0]->toWorld = controllerL * scale(mat4(1.0f), vec3(0.0114f));
		}
		else {
			balls[0]->toWorld = playerData.controllerPose[0] * scale(mat4(1.0f), vec3(0.0114f));
		}	
		balls[0]->Draw(shader, projection, view);
	}
	else {
		balls[0]->toWorld = transf
			* translate(mat4(1.0f), ball_pos[0])
			* toMat4(ball_rot[0])
			* scale(mat4(1.0f), vec3(0.0114f));
		if (ball_on[0]) {
			balls[0]->Draw(shader, projection, view);
		}
	}
	
	// ball 1-15
	for (int i = 1; i < NUMBALL; i++) {
		balls[i]->toWorld = transf 
			* translate(mat4(1.0f), ball_pos[i]) 
			* toMat4(ball_rot[i])
			* scale(mat4(1.0f), vec3(0.0114f));
		if (ball_on[i]) {
			balls[i]->Draw(shader, projection, view);
		}
	}
	

	

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


