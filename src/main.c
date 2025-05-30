// TODO: model importing, culling, camera, multiple objects, pbr.

#define GL_SILENCE_DEPRECATION
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GLFW/glfw3.h>
#include "cglm/cglm.h"

#include "../include/vertices.h"
#include "../include/standard.h"

// Settings.
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

double lastTime = 0.0f;
mat4 model, view, proj, mvp;
vec3 cameraPos = {0.0f, 0.0f, 3.0f};
vec3 cameraFront = {0.0f, 0.0f, -1.0f};
vec3 cameraUp = {0.0f, 1.0f, 0.0f}; 
vec3 cameraDirection;
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;
const float sensitivity = 0.1f;
float speed = 10;

vec3 cubePos[10] = {
    {0.0f,  0.0f,  0.0f}, 
    {2.0f,  5.0f, -15.0f}, 
    {-1.5f, -2.2f, -2.5f},  
    {-3.8f, -2.0f, -12.3f},  
    {2.4f, -0.4f, -3.5f},  
    {-1.7f,  3.0f, -7.5f},  
    {1.3f, -2.0f, -2.5f},  
    {1.5f,  2.0f, -2.5f}, 
    {1.5f,  0.2f, -1.5f}, 
    {-1.3f,  1.0f, -1.5f}  
};

double lastX = SCR_WIDTH / 2;
double lastY = SCR_HEIGHT / 2;
double deltaX, deltaY;

// Helpers.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
void showDeltaTime(GLFWwindow* window);
const char* getShaderSource(const char* name);

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Renderer", NULL, NULL);
	if (window == NULL)
	{
		printf("Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	int version = gladLoadGL(glfwGetProcAddress);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	// Default is CCW winding.
	glFrontFace(GL_CCW);

	glCullFace(GL_BACK);  

	const char* vertexShaderSource = getShaderSource("vertex_shader.txt");
	const char* fragmentShaderSource = getShaderSource("fragment_shader.txt");

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Check for shader compile errors.
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
	}
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
	}

	// Link shaders.
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Check for linking errors.
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	unsigned int VBO, VAO;//, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Texture coord attribute.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Texture setup.
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	unsigned char *data = stbi_load("../media/container.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		printf("ERROR: Failed to load texture\n");
	}
	stbi_image_free(data);

	// Matrix setup.
	// Model: object -> world.
	glm_mat4_identity(model);
	vec3 translate = {0.0f, 0.0f, -3.0f};
	glm_translate(model, translate);
	// View: world -> camera.
	glm_vec3_add(cameraPos, cameraFront, cameraDirection);
	glm_lookat(cameraPos, cameraDirection, cameraUp, view);
	// Projection: camera -> screen.
	float fov = glm_rad(45.0f);
	float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
	float near = 0.1f;
	float far = 100.0f;
	glm_perspective(fov, aspect, near, far, proj);
	// mvp = proj * view * model.
//	glm_mat4_mul(view, model, mvp);
//	glm_mat4_mul(proj, mvp, mvp); 
	// Other.
	vec3 xAxis = {1.0f, 0.0f, 0.0f};
	vec3 yAxis = {0.0f, 1.0f, 0.0f};
	vec3 zAxis = {0.0f, 0.0f, 1.0f};

	// Time setup.
	lastTime = glfwGetTime();

	// Camera control setup.
	float vel[] = {0, 0, 0};
	float rot[] = {0, 0, 0};
	glfwSetCursorPosCallback(window, mouse_callback);
	
	// Render loop.
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		showDeltaTime(window);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);

		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const float*)view);
		unsigned int projLoc = glGetUniformLocation(shaderProgram, "proj");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const float*)proj);

		glBindVertexArray(VAO);
		for(unsigned int i = 0; i < 10; i++) {
			mat4 model;
			glm_mat4_identity(model);
			glm_translate(model, cubePos[i]);
			glm_rotate(model, (float)glfwGetTime(), yAxis);
			//glm_mat4_mul(view, model, mvp);
			//glm_mat4_mul(proj, mvp, mvp);

			unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const float*)model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	if (vertexShaderSource != NULL) {
		free((void*)vertexShaderSource);
	}
	if (fragmentShaderSource != NULL) {
		free((void*)fragmentShaderSource);
	}

	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow *window) {
	// Close window.
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	// Movement.
	double currentTime = glfwGetTime();
	double deltaTime = (currentTime - lastTime);

	const float cameraSpeed = speed *  deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		glm_vec3_muladds(cameraFront, cameraSpeed, cameraPos);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		glm_vec3_muladds(cameraFront, -cameraSpeed, cameraPos);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		vec3 right;
		glm_vec3_cross(cameraFront, cameraUp, right);
		glm_vec3_normalize(right);
		glm_vec3_muladds(right, -cameraSpeed, cameraPos);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		vec3 right;
		glm_vec3_cross(cameraFront, cameraUp, right);
		glm_vec3_normalize(right);
		glm_vec3_muladds(right, cameraSpeed, cameraPos);
	}

	glm_vec3_add(cameraPos, cameraFront, cameraDirection);
	glm_lookat(cameraPos, cameraDirection, cameraUp, view);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// Frametime in milliseconds.
void showDeltaTime(GLFWwindow* window) {
	double currentTime = glfwGetTime();
	double deltaTime = (currentTime - lastTime) * 1000;
	char str[80];
	sprintf(str, "Renderer | %.2fms/frame", deltaTime);
	glfwSetWindowTitle(window, str);
	lastTime = currentTime;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	deltaX = xpos - lastX;
	deltaY = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	deltaX *= sensitivity;
	deltaY *= sensitivity;

	yaw += deltaX;
	pitch += deltaY;

	if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;

	cameraFront[0] = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
	cameraFront[1] = sin(glm_rad(pitch));
	cameraFront[2] = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
	glm_vec3_normalize(cameraFront);
}

const char* getShaderSource(const char* name) {
	FILE* file;
	char* text;
	int length;
	char c;
	int i = 0; 

	if (strcmp(name, "vertex_shader.txt") == 0) {
		file = fopen("../src/vertex_shader.txt", "r");
	} else if (strcmp(name, "fragment_shader.txt") == 0) {
		file = fopen("../src/fragment_shader.txt", "r");
	} else {
		printf("ERROR: Invalid shader source file name.");
		return NULL;
	}

	if (file == NULL) {
		printf("ERROR: Could not open shader source file.");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);

	text = malloc(sizeof(char) * (length + 1));

	while ((c = fgetc(file)) != EOF) {
		text[i] = c;
		i++;
	}
	text[i] = '\0';

	fclose(file);

	return text;
}

