//Renderer Matrix Library
//TODO: garbage collector, quaternions, transpose/major, special matrices, fix errors, fix malloc bug

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

extern const unsigned int size;

#include "../include/original/rml.h"

void rmlPrint(float mat[size][size]) {
	if (mat != NULL) {
		for (unsigned int i = 0; i < size; i++) {
			for (unsigned int j = 0; j < size; j++) {
				printf("%.2f ", mat[i][j]);
			}
		printf("\n");
		}
		printf("\n");
	} else printf("RML_ERROR_PRINTMATRIX: passed NULL.\n");
}

void rmlFill(float scalar, float mat[size][size]) {
	if (mat != NULL) {
		for (unsigned int i = 0; i < size; i++) {
			for (unsigned int j = 0; j < size; j++) {
				mat[i][j] = scalar;
			}
		}
	} else printf("RML_ERROR_FILLMATRIX: passed NULL.\n");
}

//Modifies 'out' to the dot product of input matrices.
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

//Modifies 'out' to a scaling matrix specified by 'vec'.
//Not uniform scale.
void rmlScaleMat(float vec[size - 1], float out[size][size]) {	
	for (unsigned int i = 0; i < size; i++) {
		for (unsigned int j = 0; j < size; j++) {
			if (i == j && i != size - 1) {
				out[i][j] = vec[i];
			} else if (i == j && i == size - 1) out[i][j] = 1.0;
			else out[i][j] = 0.0;
		}
	}
}

//Modifies 'out' to a rotation matrix specified by axis 'vec' and angle 'rad'.
//Can result in gimbal lock.
void rmlRotateMat(float vec[size - 1], double rad, float out[size][size]) {
	out[0][3] = 0.0;
	out[1][3] = 0.0; 
	out[2][3] = 0.0;
	out[3][0] = 0.0; 
	out[3][1] = 0.0;
	out[3][2] = 0.0; 
	out[3][3] = 1.0;
	out[0][0] = cos(rad) + vec[0] * vec[0] * (1 - cos(rad));
	out[0][1] = vec[0] * vec[1] * (1 - cos(rad)) - vec[2] * sin(rad);	
	out[0][2] = vec[0] * vec[2] * (1 - cos(rad)) + vec[1] * sin(rad);
	out[1][0] = vec[1] * vec[0] * (1 - cos(rad)) + vec[2] * sin(rad);
	out[1][1] = cos(rad) + vec[1] * vec[1] * (1 - cos(rad));
	out[1][2] = vec[1] * vec[2] * (1 - cos(rad)) - vec[0] * sin(rad);
	out[2][0] = vec[2] * vec[0] * (1 - cos(rad)) - vec[1] * sin(rad);
	out[2][1] = vec[2] * vec[1] * (1 - cos(rad)) + vec[0] * sin(rad);
	out[2][2] = cos(rad) + vec[2] * vec[2] * (1 - cos(rad));
}

//Modifies 'out' to translation matrix specified by 'vec'.
void rmlTranslateMat(float vec[size - 1], float out[size][size]) {
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

//Modifies 'out' to a projection matrix.
void rmlProjectMat(unsigned int h, unsigned int w, double fov, float zFar, float zNear, float out[size][size]) {
	rmlFill(0.0, out);
	out[0][0] = ((float)h / (float)w) / tan(fov / 2);
	out[1][1] = 1 / tan(fov / 2);
	out[2][2] = zFar / (zFar - zNear);
	out[3][2] = (-zFar * zNear) / (zFar - zNear);
	out[2][3] = 1.0;
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
