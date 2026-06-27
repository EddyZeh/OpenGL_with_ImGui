#include "scene.h"

unsigned int Scene::SCR_WIDTH = 0;
unsigned int Scene::SCR_HEIGHT = 0;

void Scene::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	Scene::SCR_WIDTH = width;
	Scene::SCR_HEIGHT = height;
}

Scene::Scene() : mainJ(0){}

Scene::Scene(int glfwVersionMajor, int glfwVersionMinor, const char* title, unsigned int scrWidth, unsigned int scrHeight)
		   : glfwVersionMajor(glfwVersionMajor), glfwVersionMinor(glfwVersionMinor), 
			 title(title), activeCam(-1), mainJ(0){
	Scene::SCR_WIDTH = scrWidth;
	Scene::SCR_HEIGHT = scrHeight;
}

bool Scene::init() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glfwVersionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glfwVersionMinor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(Scene::SCR_WIDTH, Scene::SCR_HEIGHT, title, NULL, NULL);
	if (window == NULL)
		return false;

	glfwMakeContextCurrent(window);

	// set GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	// set up sceen
	glViewport(0, 0, SCR_HEIGHT, SCR_HEIGHT);

	// Callback
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwSetKeyCallback(window, Keyboard::keyCallback);
	glfwSetCursorPosCallback(window, Mouse::cursorCallback);
	glfwSetMouseButtonCallback(window, Mouse::mouseButtonCallback);
	glfwSetScrollCallback(window, Mouse::mouseWheelCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	// Configure glboal OPENGL states
	glEnable(GL_DEPTH_TEST); // obstructing vertices are not rendered
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void Scene::initFrontEnd(){
	// Create IMGUI context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Style
	ImGui::StyleColorsDark();


	// Initialize backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

bool Scene::joystickPresent(){
	mainJ.update();
	if (mainJ.isPresent())
		return true;
	return false;
}

void Scene::addCam(Camera* camera){
	cameras.push_back(camera);
	activeCam++;
}

void Scene::processInput(float dt){
	if (activeCam != -1 && activeCam < cameras.size()) {
		
		// Mouse
		double dx = Mouse::getDX(), dy = Mouse::getDY();
		if (dx != 0 || dy != 0) {
			cameras[activeCam]->updateCameraDirection(dx, dy);
		}

		double scrollDY = Mouse::getScrollDY();
		if (scrollDY != 0) {
			cameras[activeCam]->updateCameraZoom(scrollDY);
		}

		// KEYBOARD =============================
		if (Keyboard::key(GLFW_KEY_ESCAPE)) {
			shouldClose();
		}
		if (Keyboard::key(GLFW_KEY_W)) {
			cameras[activeCam]->updateCameraPos(CameraDirection::FORWARD, dt);
		}

		if (Keyboard::key(GLFW_KEY_S)) {
			cameras[activeCam]->updateCameraPos(CameraDirection::BACKWARD, dt);
		}

		if (Keyboard::key(GLFW_KEY_D)) {
			cameras[activeCam]->updateCameraPos(CameraDirection::RIGHT, dt);
		}

		if (Keyboard::key(GLFW_KEY_A)) {
			cameras[activeCam]->updateCameraPos(CameraDirection::LEFT, dt);
		}

		if (Keyboard::key(GLFW_KEY_SPACE)) {
			cameras[activeCam]->updateCameraPos(CameraDirection::UP, dt);
		}

		if (Keyboard::key(GLFW_KEY_LEFT_SHIFT)) {
			cameras[activeCam]->updateCameraPos(CameraDirection::DOWN, dt);
		}

		mainJ.update();

		// JOYSTIC =============================
		float lx = mainJ.axesState(GLFW_GAMEPAD_AXIS_LEFT_X);
		float ly = -mainJ.axesState(GLFW_GAMEPAD_AXIS_LEFT_Y);


		float sensitivity = 50.0f;
		float ry = -mainJ.axesState(GLFW_GAMEPAD_AXIS_RIGHT_Y);
		float rx = mainJ.axesState(GLFW_GAMEPAD_AXIS_RIGHT_X);

		if (lx > 0.5f) {
			cameras[activeCam]->updateCameraPos(CameraDirection::RIGHT, dt);
		}

		if (lx < -0.5f) {
			cameras[activeCam]->updateCameraPos(CameraDirection::LEFT, dt);
		}

		if (ly > 0.5f) {
			cameras[activeCam]->updateCameraPos(CameraDirection::FORWARD, dt);
		}

		if (ly < -0.05f) {
			cameras[activeCam]->updateCameraPos(CameraDirection::BACKWARD, dt);
		}

		if (mainJ.buttonState(GLFW_GAMEPAD_BUTTON_A)) {
			cameras[activeCam]->updateCameraPos(CameraDirection::UP, dt);
		}

		if (mainJ.buttonState(GLFW_GAMEPAD_BUTTON_X)) {
			cameras[activeCam]->updateCameraPos(CameraDirection::DOWN, dt);
		}

		/*if (std::abs(ry) > 0.5f) {
			z += (ry * speed * dt);
		}*/


		if (std::abs(rx) > 0.1f || std::abs(ry) > 0.1f) {
			double dx = rx * sensitivity * dt;
			double dy = ry * sensitivity * dt;
			cameras[activeCam]->updateCameraDirection(dx, dy);
		}

		// set Matrices
		view = cameras[activeCam]->getViewMatrix();
		projection = glm::perspective(glm::radians(cameras[activeCam]->getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		cameraPos = cameras[activeCam]->cameraPos;
	}
}

void Scene::update() {
	glClearColor(bg[0], bg[1], bg[2], bg[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
}

void Scene::frontendNewFrame(){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Scene::backendNewFrame() {
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Scene::render(Shader& shader, bool applyLighting){
	shader.activate();
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	shader.set3Float("viewPos", cameraPos);
}

void Scene::renderIMGUI(){
	// Render ImGui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Scene::cleanup() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}

bool Scene::shouldClose() {
	return glfwWindowShouldClose(window);
}

Camera* Scene::getActiveCamera(){
	return (activeCam >= 0 && activeCam < cameras.size()) ? cameras[activeCam] : nullptr;
}

void Scene::setShouldClose(bool shouldClose){
	glfwSetWindowShouldClose(window, shouldClose);
}

void Scene::setWindowColor(float r, float g, float b, float a){
	bg[0] = r;
	bg[1] = g;
	bg[2] = b;
	bg[3] = a;
}





