//Renderer Matrix Library
//TODO: quaternions, error handling, performance improvements

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

extern const unsigned int size;

#include "../include/original/rml.h"

void rmlPrint(float mat[size][size]) {
	for (unsigned int i = 0; i < size; i++) {
		for (unsigned int j = 0; j < size; j++) {
			printf("%.2f ", mat[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void rmlFill(float scalar, float mat[size][size]) {
	for (unsigned int i = 0; i < size; i++) {
		for (unsigned int j = 0; j < size; j++) {
			mat[i][j] = scalar;
		}
	}
}

//Modifies 'out' to dot product of input matrices.
//Not commutative. Should do scaling -> rotation -> translation, which is t.(r.s).
void rmlDot(float in1[size][size], float in2[size][size], float out[size][size]) {
	float holder[size][size];
	rmlFill(0.0, holder);
	for (unsigned int i = 0; i < size; i++) {
		for (unsigned int j = 0; j < size; j++) {
			for (unsigned int k = 0; k < size; k++) {
				holder[i][j] += in1[i][k] * in2[k][j];
			}
		}
	}
	for (unsigned int i = 0; i < size; i++) {
		for (unsigned int j = 0; j < size; j++) {
			out[i][j] = holder[i][j];
		}
	}
} 

//Modifies 'out' to scaling matrix specified by 'vec'.
//Not uniform scale.
void rmlScale(float vec[size - 1], float out[size][size]) {	
	for (unsigned int i = 0; i < size; i++) {
		for (unsigned int j = 0; j < size; j++) {
			if (i == j && i != size - 1) {
				out[i][j] = vec[i];
			} else if (i == j && i == size - 1) out[i][j] = 1.0;
			else out[i][j] = 0.0;
		}
	}
}

//Modifies 'out' to rotation matrix specified by axis 'vec' and angle 'rad'.
//Can result in gimbal lock.
void rmlRotate(float vec[size - 1], double rad, float out[size][size]) {
	out[0][0] = cos(rad) + vec[0] * vec[0] * (1 - cos(rad));
	out[1][0] = vec[1] * vec[0] * (1 - cos(rad)) + vec[2] * sin(rad);
	out[2][0] = vec[2] * vec[0] * (1 - cos(rad)) - vec[1] * sin(rad);
	out[3][0] = 0; 

	out[0][1] = vec[0] * vec[1] * (1 - cos(rad)) - vec[2] * sin(rad);
	out[1][1] = cos(rad) + vec[1] * vec[1] * (1 - cos(rad));
	out[2][1] = vec[2] * vec[1] * (1 - cos(rad)) + vec[0] * sin(rad);
	out[3][1] = 0; 

	out[0][2] = vec[0] * vec[2] * (1 - cos(rad)) + vec[1] * sin(rad);
	out[1][2] = vec[1] * vec[2] * (1 - cos(rad)) - vec[0] * sin(rad); 
	out[2][2] = cos(rad) + vec[2] * vec[2] * (1 - cos(rad));
	out[3][2] = 0;

	out[0][3] = 0; 
	out[1][3] = 0; 
	out[2][3] = 0; 
	out[3][3] = 1; 
}

//Modifies 'out' to translation matrix specified by 'vec'.
void rmlTranslate(float vec[size - 1], float out[size][size]) {
	for (unsigned int i = 0; i < size; i++) {
		for (unsigned int j = 0; j < size; j++) {
			if (i == j) {
				out[i][j] = 1.0;
			} else out[i][j] = 0.0;
		}
	}
	out[0][3] = vec[0];
	out[1][3] = vec[1];
	out[2][3] = vec[2];
}

//Modifies 'out' to projection matrix.
void rmlProject(float angleOfView, int w, int h, float n, float f, float out[size][size]) { 
	float b, t, l, r; 
	float imageAspectRatio = (float)w / (float)h; 
	float scale = tan(angleOfView * 0.5 * M_PI / 180) * n; 
	r = imageAspectRatio * scale, l = -r; 
	t = scale, b = -t; 
	
	out[0][0] = 2 * n / (r - l); 
	out[1][0] = 0; 
	out[2][0] = 0; 
	out[3][0] = 0; 

	out[0][1] = 0; 
	out[1][1] = 2 * n / (t - b); 
	out[2][1] = 0; 
	out[3][1] = 0; 

	out[0][2] = (r + l) / (r - l); 
	out[1][2] = (t + b) / (t - b); 
	out[2][2] = -(f + n) / (f - n); 
	out[3][2] = -1; 

	out[0][3] = 0; 
	out[1][3] = 0; 
	out[2][3] = -2 * f * n / (f - n); 
	out[3][3] = 0; 
}

void rmlNormaliseVec(float vec[size - 1]) {
	float length = rmlLengthOfVec(vec);
	vec[0] = vec[0] / length;
	vec[1] = vec[1] / length;
	vec[2] = vec[2] / length;
}

float rmlLengthOfVec(float vec[size - 1]) {
	float length = 0.0;
	for (unsigned int i = 0; i < size; i++) {
		length += vec[i]*vec[i];
	}
	return sqrt(length);
}
