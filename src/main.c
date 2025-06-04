// TODO: infinite chunks, UV lookup, chunk border culling, greedy meshing, world saving/seeds, gui, fog, biomes, up/down camera, other blocks, breaking/placing, vertex data optimisation, checks per face, multithreading, multiplayer, pbr.

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
int w, h;
float aspect;
float fov;
float near = 0.1f;
float far = 1000.0f;

#define VERTEX_SIZE 5
#define VERTICES_PER_BLOCK 36
#define DATA_PER_BLOCK 180

float uMin[6], uMax[6], vMin[6], vMax[6];

enum BlockType {
	AIR,
	DIRT,
	GRASS,
	STONE
};

#define chunkSize 16
#define chunksInX 10
#define chunksInY 5
#define chunksInZ 10
#define topsoilDepth 4
#define perlinFrequency 0.02
#define perlinAmplitude 40
#define perlinYDisp 40
#define renderDist 500
unsigned int numChunks = chunksInX * chunksInY * chunksInZ;
unsigned int blocksPerChunk = chunkSize * chunkSize * chunkSize;
// World array.
char wa[chunksInX][chunksInY][chunksInZ][chunkSize][chunkSize][chunkSize];
int atlasSize;
#define numBlockTypes 4 // Including AIR.
// Vertex array array.
float vaa[numBlockTypes][DATA_PER_BLOCK];

bool menu = false;
bool menuLastState = false;
bool menuThisState = false;

bool wire = false;
bool wireLastState = false;
bool wireThisState = false;

unsigned int VBO, VAO;//, EBO;

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

double lastX = SCR_WIDTH / 2;
double lastY = SCR_HEIGHT / 2;
double deltaX, deltaY;

// Helpers.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
void showDeltaTime(GLFWwindow* window);
const char* getShaderSource(const char* name);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void addChunk(float* dst, unsigned int chunkIndex, unsigned int offset[3]);
void addCube(float* dst, float offset[3], int index, enum BlockType curBT);
void writeBlockTypeBin(enum BlockType blockType);
int readBlockTypeBin(enum BlockType blockType);
unsigned int getHeight(int x, int z);

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
	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	gladLoadGL(glfwGetProcAddress);

	// Default is CCW winding.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
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
		printf("Vertex: ERROR::SHADER::VERTEX::COMPILATION_FAILED %s\n", infoLog);
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
		printf("Fragment: ERROR::SHADER::PROGRAM::LINKING_FAILED %s\n", infoLog);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	// Texture setup.
	unsigned int texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	int width, height, nrChannels;
	unsigned char *data = stbi_load("../media/atlas.png", &width, &height, &nrChannels, 0);
	if (width != height) printf("ERROR: Texture atlas not square\n");
	else atlasSize = width;
	if (data) {
		unsigned int format = nrChannels == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
	fov = glm_rad(45.0f);
	glfwGetWindowSize(window, &w, &h);
	aspect = (float)w / (float)h;
	glm_perspective(fov, aspect, near, far, proj);

	// Time setup.
	lastTime = glfwGetTime();

	// Camera control setup.
	glfwSetCursorPosCallback(window, mouse_callback);
	
	// Mesh batching setup.
	for (int i = 1; i < numBlockTypes; i++) readBlockTypeBin(i);
	unsigned int totalVerticesSize = sizeof(float) * DATA_PER_BLOCK * blocksPerChunk * numChunks;
	float* combinedVertices = malloc(totalVerticesSize);

	for (unsigned int cx = 0; cx < chunksInX; cx++) {
		for (unsigned int cy = 0; cy < chunksInY; cy++) {
			for (unsigned int cz = 0; cz < chunksInZ; cz++) {
				unsigned int chunkIndex = cx * chunksInY * chunksInZ + cy * chunksInZ + cz;
				unsigned int o[3] = {cx, cy, cz};
				addChunk(combinedVertices, chunkIndex, o);
			}
		}
	}

	// Vertices setup.	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, totalVerticesSize, combinedVertices, GL_STATIC_DRAW);
	// Position attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Texture coord attribute.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	free(combinedVertices);

	// Render loop.
	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		showDeltaTime(window);

		if (menu) glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		else glClearColor(0.4f, 0.65f, 1.0f, 0.8f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shaderProgram);

		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const float*)view);
		unsigned int projLoc = glGetUniformLocation(shaderProgram, "proj");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const float*)proj);
		unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const float*)model);

		glBindVertexArray(VAO);

		if (wire) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (!menu) {
			glDrawArrays(GL_TRIANGLES, 0, VERTICES_PER_BLOCK * blocksPerChunk * numChunks);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}
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

	// Menu. Currently only mouse toggle.
	if (menu) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwGetWindowSize(window, &w, &h);
		aspect = (float)w / (float)h;
		glm_perspective(fov, aspect, near, far, proj);
	} else {

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		// Movement.
		double currentTime = glfwGetTime();
		double deltaTime = (currentTime - lastTime);

		float cameraSpeed = speed *  deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) cameraSpeed *= 2;
			glm_vec3_muladds(cameraFront, cameraSpeed, cameraPos);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) cameraSpeed *= 2;
			glm_vec3_muladds(cameraFront, -cameraSpeed, cameraPos);
		}
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

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	(void)window;
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
	(void)window;
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	(void)window;
	scancode++; mods++; // Silence warning.

	// Toggle menu (currently black screen) and mouse.
	// Can use to go fullscreen.
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		menuLastState = menuThisState;
		menuThisState = true;
	} else menuThisState = false;
	if (menuLastState != menuThisState) menu = (menu == true) ? false : true;

	// Toggle wireframe.
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		wireLastState = wireThisState;
		wireThisState = true;
	} else wireThisState = false;
	if (wireLastState != wireThisState) wire = (wire == true) ? false : true;
}

void addChunk(float* dst, unsigned int chunkIndex, unsigned int o[3]) {
	for (int x = 0; x < chunkSize; x++) {
		for (int y = 0; y < chunkSize; y++) {
			for (int z = 0; z < chunkSize; z++) {
				int globalX = x + chunkSize * o[0];
				int globalY = y + chunkSize * o[1];
				int globalZ = z + chunkSize * o[2];

				int h = getHeight(globalX, globalZ);

				if (globalY > h) {
					wa[o[0]][o[1]][o[2]][x][y][z] = AIR;
				} else if (globalY == h) {
					wa[o[0]][o[1]][o[2]][x][y][z] = GRASS;
				} else if (globalY >= h - topsoilDepth + 1) {
					wa[o[0]][o[1]][o[2]][x][y][z] = DIRT;
				} else {
					wa[o[0]][o[1]][o[2]][x][y][z] = STONE;
				}
			}
		}
	}


	// Fill all chunks with stone.
 	//memset(infoPerChunk, STONE, sizeof(infoPerChunk));

	enum BlockType curBT;
	
	for (int x = 0; x < chunkSize; x++) {
		for (int y = 0; y < chunkSize; y++) {
			for (int z = 0; z < chunkSize; z++) {
				if (wa[o[0]][o[1]][o[2]][x][y][z] == AIR) continue;
				if (x <= chunkSize - 2 && x > 0 && y <= chunkSize - 2 && y > 0 && z <= chunkSize - 2 && z > 0 &&
					wa[o[0]][o[1]][o[2]][x + 1][y][z] != AIR && 
					wa[o[0]][o[1]][o[2]][x - 1][y][z] != AIR &&
					wa[o[0]][o[1]][o[2]][x][y + 1][z] != AIR &&
					wa[o[0]][o[1]][o[2]][x][y - 1][z] != AIR &&
					wa[o[0]][o[1]][o[2]][x][y][z + 1] != AIR &&
					wa[o[0]][o[1]][o[2]][x][y][z - 1] != AIR) continue;
				switch (wa[o[0]][o[1]][o[2]][x][y][z]) {
					case DIRT: curBT = DIRT; break;
					case GRASS: curBT = GRASS; break;
					case STONE: curBT = STONE; break;
					default: break;
					
				}
				unsigned int localIndex = x * chunkSize * chunkSize + y * chunkSize + z;
				unsigned int globalIndex = chunkIndex * chunkSize * chunkSize * chunkSize + localIndex;

				float oCube[3] = {(float)(x + o[0] * chunkSize), (float)(y + o[1] * chunkSize), (float)(z + o[2] * chunkSize)};

				addCube(dst, oCube, globalIndex, curBT);
			}
		}
	}
}

void addCube(float* dst, float o[3], int index, enum BlockType curBT) {
    for (int i = 0; i < VERTICES_PER_BLOCK; ++i) {
        int srcIndex = i * VERTEX_SIZE;
        int dstIndex = index * DATA_PER_BLOCK + srcIndex;

        dst[dstIndex + 0] = vaa[curBT][srcIndex + 0] + o[0];
        dst[dstIndex + 1] = vaa[curBT][srcIndex + 1] + o[1];
        dst[dstIndex + 2] = vaa[curBT][srcIndex + 2] + o[2];
        dst[dstIndex + 3] = vaa[curBT][srcIndex + 3]; // u
        dst[dstIndex + 4] = vaa[curBT][srcIndex + 4]; // v
    }
}

//void setUV(enum BlockType blockType) {
//	float tileSize = 1.0f / (atlasSize / 32.0f);
//	switch (blockType) {
//		case DIRT:
//			uMin[0] = 2 * tileSize;
//			uMax[0] = uMin[0] + tileSize;
//			vMin[0] = 0 * tileSize;
//			vMax[0] = vMin[0] + tileSize;
//
//			for (int i = 1; i < 6; i++) {
//				uMin[i] = uMin[0];
//				uMax[i] = uMax[0];
//				vMin[i] = vMin[0];
//				vMax[i] = vMax[0];
//			}
//
//			break;
//		case GRASS:
//			uMin[0] = 1 * tileSize;
//			uMax[0] = uMin[0] + tileSize;
//			vMin[0] = 0 * tileSize;
//			vMax[0] = vMin[0] + tileSize;
//
//			for (int i = 1; i < 4; i++) {
//				uMin[i] = uMin[0];
//				uMax[i] = uMax[0];
//				vMin[i] = vMin[0];
//				vMax[i] = vMax[0];
//			}
//
//			uMin[4] = 2 * tileSize;
//			uMax[4] = uMin[4] + tileSize;
//			vMin[4] = 0 * tileSize;
//			vMax[4] = vMin[4] + tileSize;
//
//			uMin[5] = 0 * tileSize;
//			uMax[5] = uMin[5] + tileSize;
//			vMin[5] = 0 * tileSize;
//			vMax[5] = vMin[5] + tileSize;
//
//			break;
//		case STONE:
//			uMin[0] = 0 * tileSize;
//			uMax[0] = uMin[0] + tileSize;
//			vMin[0] = 1 * tileSize;
//			vMax[0] = vMin[0] + tileSize;
//
//			for (int i = 1; i < 6; i++) {
//				uMin[i] = uMin[0];
//				uMax[i] = uMax[0];
//				vMin[i] = vMin[0];
//				vMax[i] = vMax[0];
//			}
//
//			break;
//		default:
//			break;
//	}
//
//	for (int i = 0; i < DATA_PER_BLOCK; i++) {
//		if (i % 5 < 3) continue;
//		switch (i) {
//			// Back face.
//			case 3: vertices[i] = uMin[0]; break;
//			case 4: vertices[i] = vMax[0]; break;
//			case 8: vertices[i] = uMax[0]; break;
//			case 9: vertices[i] = vMin[0]; break;
//			case 13: vertices[i] = uMax[0]; break;
//			case 14: vertices[i] = vMax[0]; break;
//			case 18: vertices[i] = uMax[0]; break;
//			case 19: vertices[i] = vMin[0]; break;
//			case 23: vertices[i] = uMin[0]; break;
//			case 24: vertices[i] = vMax[0]; break;
//			case 28: vertices[i] = uMin[0]; break;
//			case 29: vertices[i] = vMin[0]; break;
//
//			// Front face.
//			case 33: vertices[i] = uMin[1]; break;
//			case 34: vertices[i] = vMax[1]; break;
//			case 38: vertices[i] = uMax[1]; break;
//			case 39: vertices[i] = vMax[1]; break;
//			case 43: vertices[i] = uMax[1]; break;
//			case 44: vertices[i] = vMin[1]; break;
//			case 48: vertices[i] = uMax[1]; break;
//			case 49: vertices[i] = vMin[1]; break;
//			case 53: vertices[i] = uMin[1]; break;
//			case 54: vertices[i] = vMin[1]; break;
//			case 58: vertices[i] = uMin[1]; break;
//			case 59: vertices[i] = vMax[1]; break;
//
//			// Left face.
//			case 63: vertices[i] = uMin[2]; break;
//			case 64: vertices[i] = vMin[2]; break;
//			case 68: vertices[i] = uMax[2]; break;
//			case 69: vertices[i] = vMin[2]; break;
//			case 73: vertices[i] = uMax[2]; break;
//			case 74: vertices[i] = vMax[2]; break;
//			case 78: vertices[i] = uMax[2]; break;
//			case 79: vertices[i] = vMax[2]; break;
//			case 83: vertices[i] = uMin[2]; break;
//			case 84: vertices[i] = vMax[2]; break;
//			case 88: vertices[i] = uMin[2]; break;
//			case 89: vertices[i] = vMin[2]; break;
//
//			// Right face.
//			case 93: vertices[i] = uMin[3]; break;
//			case 94: vertices[i] = vMin[3]; break;
//			case 98: vertices[i] = uMax[3]; break;
//			case 99: vertices[i] = vMax[3]; break;
//			case 103: vertices[i] = uMax[3]; break;
//			case 104: vertices[i] = vMin[3]; break;
//			case 108: vertices[i] = uMax[3]; break;
//			case 109: vertices[i] = vMax[3]; break;
//			case 113: vertices[i] = uMin[3]; break;
//			case 114: vertices[i] = vMin[3]; break;
//			case 118: vertices[i] = uMin[3]; break;
//			case 119: vertices[i] = vMax[3]; break;
//
//			// Bottom face.
//			case 123: vertices[i] = uMin[4]; break;
//			case 124: vertices[i] = vMin[4]; break;
//			case 128: vertices[i] = uMax[4]; break;
//			case 129: vertices[i] = vMin[4]; break;
//			case 133: vertices[i] = uMax[4]; break;
//			case 134: vertices[i] = vMax[4]; break;
//			case 138: vertices[i] = uMax[4]; break;
//			case 139: vertices[i] = vMax[4]; break;
//			case 143: vertices[i] = uMin[4]; break;
//			case 144: vertices[i] = vMax[4]; break;
//			case 148: vertices[i] = uMin[4]; break;
//			case 149: vertices[i] = vMin[4]; break;
//
//			// Top face.
//			case 153: vertices[i] = uMin[5]; break;
//			case 154: vertices[i] = vMin[5]; break;
//			case 158: vertices[i] = uMax[5]; break;
//			case 159: vertices[i] = vMax[5]; break;
//			case 163: vertices[i] = uMax[5]; break;
//			case 164: vertices[i] = vMin[5]; break;
//			case 168: vertices[i] = uMax[5]; break;
//			case 169: vertices[i] = vMax[5]; break;
//			case 173: vertices[i] = uMin[5]; break;
//			case 174: vertices[i] = vMin[5]; break;
//			case 178: vertices[i] = uMin[5]; break;
//			case 179: vertices[i] = vMax[5]; break;
//			default: break;
//		}
//	}
//}

void writeBlockTypeBin(enum BlockType blockType) {
	float tileSize = 1.0f / (atlasSize / 32.0f);
	char* fileName; 
	switch (blockType) {
		case DIRT:
			uMin[0] = 2 * tileSize;
			uMax[0] = uMin[0] + tileSize;
			vMin[0] = 0 * tileSize;
			vMax[0] = vMin[0] + tileSize;

			for (int i = 1; i < 6; i++) {
				uMin[i] = uMin[0];
				uMax[i] = uMax[0];
				vMin[i] = vMin[0];
				vMax[i] = vMax[0];
			}

			fileName = "dirt.bin";

			break;
		case GRASS:
			uMin[0] = 1 * tileSize;
			uMax[0] = uMin[0] + tileSize;
			vMin[0] = 0 * tileSize;
			vMax[0] = vMin[0] + tileSize;

			for (int i = 1; i < 4; i++) {
				uMin[i] = uMin[0];
				uMax[i] = uMax[0];
				vMin[i] = vMin[0];
				vMax[i] = vMax[0];
			}

			uMin[4] = 2 * tileSize;
			uMax[4] = uMin[4] + tileSize;
			vMin[4] = 0 * tileSize;
			vMax[4] = vMin[4] + tileSize;

			uMin[5] = 0 * tileSize;
			uMax[5] = uMin[5] + tileSize;
			vMin[5] = 0 * tileSize;
			vMax[5] = vMin[5] + tileSize;

			fileName = "grass.bin";

			break;
		case STONE:
			uMin[0] = 0 * tileSize;
			uMax[0] = uMin[0] + tileSize;
			vMin[0] = 1 * tileSize;
			vMax[0] = vMin[0] + tileSize;

			for (int i = 1; i < 6; i++) {
				uMin[i] = uMin[0];
				uMax[i] = uMax[0];
				vMin[i] = vMin[0];
				vMax[i] = vMax[0];
			}

			fileName = "stone.bin";

			break;
		default:
			break;
	}

	float arrayToFile[DATA_PER_BLOCK];
	for (int i = 0; i < DATA_PER_BLOCK; i++) arrayToFile[i] = vertices[i];

	for (int i = 0; i < DATA_PER_BLOCK; i++) {
		if (i % 5 < 3) continue;
		switch (i) {
			// Back face.
			case 3: arrayToFile[i] = uMin[0]; break;
			case 4: arrayToFile[i] = vMax[0]; break;
			case 8: arrayToFile[i] = uMax[0]; break;
			case 9: arrayToFile[i] = vMin[0]; break;
			case 13: arrayToFile[i] = uMax[0]; break;
			case 14: arrayToFile[i] = vMax[0]; break;
			case 18: arrayToFile[i] = uMax[0]; break;
			case 19: arrayToFile[i] = vMin[0]; break;
			case 23: arrayToFile[i] = uMin[0]; break;
			case 24: arrayToFile[i] = vMax[0]; break;
			case 28: arrayToFile[i] = uMin[0]; break;
			case 29: arrayToFile[i] = vMin[0]; break;

			// Front face.
			case 33: arrayToFile[i] = uMin[1]; break;
			case 34: arrayToFile[i] = vMax[1]; break;
			case 38: arrayToFile[i] = uMax[1]; break;
			case 39: arrayToFile[i] = vMax[1]; break;
			case 43: arrayToFile[i] = uMax[1]; break;
			case 44: arrayToFile[i] = vMin[1]; break;
			case 48: arrayToFile[i] = uMax[1]; break;
			case 49: arrayToFile[i] = vMin[1]; break;
			case 53: arrayToFile[i] = uMin[1]; break;
			case 54: arrayToFile[i] = vMin[1]; break;
			case 58: arrayToFile[i] = uMin[1]; break;
			case 59: arrayToFile[i] = vMax[1]; break;

			// Left face.
			case 63: arrayToFile[i] = uMin[2]; break;
			case 64: arrayToFile[i] = vMin[2]; break;
			case 68: arrayToFile[i] = uMax[2]; break;
			case 69: arrayToFile[i] = vMin[2]; break;
			case 73: arrayToFile[i] = uMax[2]; break;
			case 74: arrayToFile[i] = vMax[2]; break;
			case 78: arrayToFile[i] = uMax[2]; break;
			case 79: arrayToFile[i] = vMax[2]; break;
			case 83: arrayToFile[i] = uMin[2]; break;
			case 84: arrayToFile[i] = vMax[2]; break;
			case 88: arrayToFile[i] = uMin[2]; break;
			case 89: arrayToFile[i] = vMin[2]; break;

			// Right face.
			case 93: arrayToFile[i] = uMin[3]; break;
			case 94: arrayToFile[i] = vMin[3]; break;
			case 98: arrayToFile[i] = uMax[3]; break;
			case 99: arrayToFile[i] = vMax[3]; break;
			case 103: arrayToFile[i] = uMax[3]; break;
			case 104: arrayToFile[i] = vMin[3]; break;
			case 108: arrayToFile[i] = uMax[3]; break;
			case 109: arrayToFile[i] = vMax[3]; break;
			case 113: arrayToFile[i] = uMin[3]; break;
			case 114: arrayToFile[i] = vMin[3]; break;
			case 118: arrayToFile[i] = uMin[3]; break;
			case 119: arrayToFile[i] = vMax[3]; break;

			// Bottom face.
			case 123: arrayToFile[i] = uMin[4]; break;
			case 124: arrayToFile[i] = vMin[4]; break;
			case 128: arrayToFile[i] = uMax[4]; break;
			case 129: arrayToFile[i] = vMin[4]; break;
			case 133: arrayToFile[i] = uMax[4]; break;
			case 134: arrayToFile[i] = vMax[4]; break;
			case 138: arrayToFile[i] = uMax[4]; break;
			case 139: arrayToFile[i] = vMax[4]; break;
			case 143: arrayToFile[i] = uMin[4]; break;
			case 144: arrayToFile[i] = vMax[4]; break;
			case 148: arrayToFile[i] = uMin[4]; break;
			case 149: arrayToFile[i] = vMin[4]; break;

			// Top face.
			case 153: arrayToFile[i] = uMin[5]; break;
			case 154: arrayToFile[i] = vMin[5]; break;
			case 158: arrayToFile[i] = uMax[5]; break;
			case 159: arrayToFile[i] = vMax[5]; break;
			case 163: arrayToFile[i] = uMax[5]; break;
			case 164: arrayToFile[i] = vMin[5]; break;
			case 168: arrayToFile[i] = uMax[5]; break;
			case 169: arrayToFile[i] = vMax[5]; break;
			case 173: arrayToFile[i] = uMin[5]; break;
			case 174: arrayToFile[i] = vMin[5]; break;
			case 178: arrayToFile[i] = uMin[5]; break;
			case 179: arrayToFile[i] = vMax[5]; break;
			default: break;
		}
	}
	FILE* f = fopen(fileName, "wb");
	fwrite(arrayToFile, sizeof(float), DATA_PER_BLOCK, f);
	fclose(f);
}

int readBlockTypeBin(enum BlockType blockType) {
	char* fileName; 
	switch (blockType) {
		case DIRT:
			fileName = "dirt.bin";
			break;
		case GRASS:
			fileName = "grass.bin";
			break;
		case STONE:
			fileName = "stone.bin";
			break;
		default:
			break;
	}

	float arrayFromFile[DATA_PER_BLOCK];

	FILE* f = fopen(fileName, "rb");
	if (!f) {
		writeBlockTypeBin(blockType);
		f = fopen(fileName, "rb");
	}
	fread(arrayFromFile, sizeof(int), DATA_PER_BLOCK, f);
	fclose(f);

	for (int i = 0; i < DATA_PER_BLOCK; i++) vaa[blockType][i] = arrayFromFile[i];

	return 1;
}

unsigned int getHeight(int x, int z) {
	vec3 pos = {(float)x, 0.0f, (float)z};
	glm_vec3_scale(pos, perlinFrequency, pos);
	float noise = glm_perlin_vec3(pos);
	return (unsigned int)((noise + 1.0f) * 0.5f * perlinAmplitude + perlinYDisp);
}
