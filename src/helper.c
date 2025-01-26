#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// reads shader text file into c string
const char* getShaderSource(const char* name) {
	FILE* file;
	char* text;
	int length;
	char c;
	int i = 0; 

	if (strcmp(name, "vertexShader.txt") == 0) {
		file = fopen("../src/vertexShader.txt", "r");
	} else if (strcmp(name, "fragmentShader.txt") == 0) {
		file = fopen("../src/fragmentShader.txt", "r");
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
