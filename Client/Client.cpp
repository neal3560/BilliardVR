
#include "Scene.h"
using namespace std;
int player_id = 0;

  ExampleApp::ExampleApp()
  {
	  client = new rpc::client("128.54.70.52", 3560);
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
  {	  // ============ call server ===================//
	  Send self_info;
	  self_info.head_pos = ovr::toGlm(eyePoses[0].Position);
	  info = client->call("echo", player_id, self_info).as<Receive>();

	  cout << info.to_string() << endl;
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
	  // Get the state of hand poses
	  ovrPoseStatef rightHandPoseState = hmdState.HandPoses[ovrHand_Right];
	  rightHandPosition = ovr::toGlm(rightHandPoseState.ThePose.Position);
	  rightHandRotation = ovr::toGlm(ovrQuatf(rightHandPoseState.ThePose.Orientation));

  }

  void ExampleApp::renderScene(const glm::mat4& projection, const glm::mat4& headPose)
  {
	mat4 controller = translate(mat4(1.0f), rightHandPosition) * mat4_cast(rightHandRotation);

	// request to server to get object's position
	

	scene->render(projection, inverse(headPose), controller, info);
  }

 

Scene::Scene()
{
	// Shader Program 
	shader = LoadShaders("shader.vert", "shader.frag");
	
	hand = new Model("models/hand.obj", false);
	head = new Model("models/sphere.obj", false);

	// 10m wide sky box: size doesn't matter though
	skybox = std::make_unique<Skybox>("skybox");
	skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));

	table = std::make_unique<Cube>();
	table->toWorld = translate(mat4(1.0f), vec3(-0.5f, 0.0f, -1.4f)) * scale(mat4(1.0f), vec3(0.35f, 0.01f, 0.7f));
	table->mode = 0;
}

void Scene::render(const mat4& projection, const mat4& view, const mat4 controller, const Receive & info)
{	
	//skybox
	skybox->draw(shader, projection, view);

	//table
	table->draw(shader, projection, view);

	//opponent
	head->toWorld = translate(mat4(1.0f), info.head_pos) * scale(mat4(1.0f), vec3(0.04f));
	head->Draw(shader, projection, view);

	//render the controller
	hand->toWorld = controller * glm::rotate(mat4(1.0f), 3.14f * 0.5f, vec3(0, 1, 0)) * glm::rotate(mat4(1.0f), 3.14f, vec3(0, 0, 1)) * scale(mat4(1.0f), vec3(0.0022f));
	hand->Draw(shader, projection, view);


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


