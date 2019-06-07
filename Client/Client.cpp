
#include "Scene.h"
using namespace std;
int player_id = 0;


  ExampleApp::ExampleApp()
  {
	  client = new rpc::client("localhost", 3560);
	  std::cout << "Connected" << std::endl;

	  LT_pre = false;
	  RT_pre = false;

	  hand_mode = false;
	  freeze_mode = false;
	  debug_mode = false;

	  cube_translation = vec3(0.0f, 0.0f, 0.0f);
	  cube_size = 30.0f;
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
	  debug_mode = A;// && hand_mode;
	  // B button
	  freeze_mode = inputState.Buttons & ovrButton_B;
	  // right hand trigger
	  hand_mode = inputState.HandTrigger[ovrHand_Right] > 0.5f;

	  // LT Button
	  bool LT_cur = inputState.Buttons & ovrButton_LThumb;
	  if (!LT_pre && LT_cur) {
		  cube_translation = vec3(0.0f, 0.0f, 0.0f);
	  }
	  LT_pre = LT_cur;

	  // LT stick
	  vec2 LT_stick = ovr::toGlm(inputState.Thumbstick[0]);
	  if (length(LT_stick) < 0.1f) {
		  LT_stick = vec2(0.0f, 0.0f);
	  }
	  cube_translation[0] += LT_stick[0] * 0.03f;
	  cube_translation[2] -= LT_stick[1] * 0.03f;

	  // RT Button
	  bool RT_cur = inputState.Buttons & ovrButton_RThumb;
	  if (!RT_pre && RT_cur) {
		  cube_size = 30.0f;
	  }
	  RT_pre = RT_cur;

	  // RT stick
	  vec2 RT_stick = ovr::toGlm(inputState.Thumbstick[1]);
	  if (length(RT_stick) < 0.1f) {
		  RT_stick = vec2(0.0f, 0.0f);
	  }
	  cube_translation[1] += RT_stick[1] * 0.03f;
	  float cube_size_changed = cube_size + RT_stick[0] * 0.5f;
	  if (cube_size_changed > 1.0f && cube_size_changed < 100.0f) {
		  cube_size = cube_size_changed;
	  }

	  // ================= controller location ==================
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

	  // ============ connect server ===================//
	  ovrTrackingState ts = ovr_GetTrackingState(_session, ovr_GetTimeInSeconds(), ovrTrue);
	  ovrPosef headpose = ts.HeadPose.ThePose;
	  
	  ClientData data;  
	  data.head_pos = ovr::toGlm(headpose.Position);    //0.5f * (ovr::toGlm(eyePoses[0].Position) + ovr::toGlm(eyePoses[0].Position));
	  data.head_rot = ovr::toGlm(headpose.Orientation);
	  for (int i = 0; i < 2; i++) {
		  data.controller_location[i] = hand_pos[i];
		  data.controller_rotation[i] = hand_rot[i];
	  }
	  data.hit = A;
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

  void ExampleApp::renderScene(const glm::mat4& projection, const glm::mat4& headPose)
  {
	mat4 left_controller = translate(mat4(1.0f), hand_pos[0]) * mat4_cast(hand_rot[0]);
	mat4 right_controller = translate(mat4(1.0f), hand_pos[1]) * mat4_cast(hand_rot[1]);
	scene->render(projection, inverse(headPose), left_controller, right_controller, playerData, ball_pos, ball_rot, ball_on);
  }

 

Scene::Scene()
{
	// Shader Program 
	shader = LoadShaders("shader.vert", "shader.frag");
	
	// player model
	hand = new Model("models/hand.obj", false);
	head = new Model("models/sphere.obj", false);

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

	fabric->mode = 1;
	fabric->toWorld = translate(mat4(1.0f), vec3(0.0f, -0.925f, 0.0f)) * transf * scale(mat4(1.0f), vec3(0.012f));

	base->mode = 1;
	base->toWorld = translate(mat4(1.0f), vec3(0.0f, -0.925f, 0.0f)) * transf * scale(mat4(1.0f), vec3(0.012f));

	// 10m wide sky box: size doesn't matter though
	skybox = std::make_unique<Skybox>("skybox");
	skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));

	// cue
	stick = std::make_unique<Cube>();

	// load texture
	diffuse_fabric = TextureFromFile("Fabric_Diffuse.png", "texture");
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, diffuse_fabric);

	specular_fabric = TextureFromFile("Fabric_Glossiness.png", "texture");
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, specular_fabric);

	diffuse_base = TextureFromFile("Table_Diffuse.png", "texture");
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, diffuse_base);

	specular_base = TextureFromFile("Table_Glossiness.png", "texture");
	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, specular_base);

	diffuse_ball = TextureFromFile("Balls_Diffuse.png", "texture");
	glActiveTexture(GL_TEXTURE0 + 5);
	glBindTexture(GL_TEXTURE_2D, diffuse_ball);

	specular_ball = TextureFromFile("Balls_Glossiness.png", "texture");
	glActiveTexture(GL_TEXTURE0 + 6);
	glBindTexture(GL_TEXTURE_2D, specular_ball);

	
}

void Scene::render(const mat4& projection, const mat4& view, const mat4 controllerL, const mat4 controllerR, const PlayerData & playerData, vec3 * ball_pos, quat * ball_rot, bool * ball_on)
{	
	float lightransf[4 * NUMLIGHT];
	for (int i = 0; i < NUMLIGHT; i++) {
		vec4 lightposAfter = view * transf * vec4(lightposn[4 * i], lightposn[4 * i + 1], lightposn[4 * i + 2], lightposn[4 * i + 3]);
		lightransf[4 * i] = lightposAfter[0];
		lightransf[4 * i + 1] = lightposAfter[1];
		lightransf[4 * i + 2] = lightposAfter[2];
		lightransf[4 * i + 3] = lightposAfter[3];
	}

	// lights
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

	// stick
	//stick->mode = 3;
	//stick->toWorld = controller * scale(mat4(1.0f), vec3(0.01f,1.0f,0.01f));
	//stick->draw(shader, projection, view);

	// table
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, diffuse_fabric);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, specular_fabric);
	glUniform1i(glGetUniformLocation(shader, "texture_diffuse"), 1);
	glUniform1i(glGetUniformLocation(shader, "texture_specular"), 2);
	fabric->Draw(shader, projection, view);

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


