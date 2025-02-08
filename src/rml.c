//Renderer Matrix Library
//TODO: garbage collector, quaternions, transpose/major, special matrices, fix errors, fix malloc bug

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "../include/original/rml.h"

//Returns pointer to row*col matrix initialised with zeros.
//Dynamically allocates. User must free with rmlFreeMatrix.
rmlMatrix* rmlAllocateMatrix(unsigned int row, unsigned int col) {
	if (row == 0 || col == 0) {
		printf("RML_ERROR_ALLOCATEMATRIX: row == 0 || col == 0.\n");
		return NULL;
	}
	rmlMatrix* mat = (rmlMatrix*)malloc(sizeof(rmlMatrix));
	if (mat == NULL) {
		printf("RML_ERROR_ALLOCATEMATRIX: allocation failed.\n");
		return NULL;
	}
	float** val = (float**)calloc(row, sizeof(float*));
	if (val == NULL) {
		free(mat);
		printf("RML_ERROR_ALLOCATEMATRIX: allocation failed.\n");
		return NULL;
	}
	for (unsigned int i = 0; i < row; i++) {
		val[i] = (float*)calloc(col, sizeof(float));
		if (val[i] == NULL) {
			for (unsigned int j = 0; j < i; j++) {
				free(val[j]);
			}
			free(val);
			free(mat);
			printf("RML_ERROR_ALLOCATEMATRIX: allocation failed.\n");
			return NULL;
		}
	}
	mat->row = row;
	mat->col = col;
	mat->val = val;
	return mat;
}

void rmlFreeMatrix(rmlMatrix* mat) {
	if (mat != NULL) {
		for (unsigned int i = 0; i < mat->row; i++) {
			free(mat->val[i]);
		}
		free(mat->val);
		free(mat);
	} else printf("RML_ERROR_FREEMATRIX: passed NULL.\n");
}

void rmlPrintMatrix(rmlMatrix* mat) {
	if (mat != NULL) {
		for (unsigned int i = 0; i < mat->row; i++) {
			for (unsigned int j = 0; j < mat->col; j++) {
				printf("%.2f ", mat->val[i][j]);
			}
		printf("\n");
		}
		printf("\n");
	} else printf("RML_ERROR_PRINTMATRIX: passed NULL.\n");
}

rmlVector* rmlAllocateVector(unsigned int size) {
	if (size == 0) {
		printf("RML_ERROR_ALLOCATEVECTOR: size == 0.\n");
		return NULL;
	}
	rmlVector* vec = (rmlVector*)malloc(sizeof(rmlVector));
	if (vec == NULL) {
		printf("RML_ERROR_ALLOCATEVECTOR: allocation failed.\n");
		return NULL;
	}
	float* val = (float*)calloc(size, sizeof(float));
	if (val == NULL) {
		free(vec);
		printf("RML_ERROR_ALLOCATEVECTOR: allocation failed.\n");
		return NULL;
	}
	vec->size = size;
	vec->val = val;
	return vec;
}

void rmlFreeVector(rmlVector* vec) {
	if (vec != NULL) {
		free(vec->val);
		free(vec);
	} else printf("RML_ERROR_FREEVECTOR: passed NULL.\n");
}

void rmlPrintVector(rmlVector* vec) {
	if (vec != NULL) {
		for (unsigned int i = 0; i < vec->size; i++) {
			printf("%.2f\n", vec->val[i]);
		}
		printf("\n");
	} else printf("RML_ERROR_PRINTVECTOR: passed NULL.\n");
}

//Returns pointer to matrix that is dot product of input matrices.
//If input is m*n.n*p, output is m*p.
//Not commutative. Advised to do scaling -> rotation -> translation, which is t.(r.s).
//Dynamically allocates. User must free with rmlFreeMatrix.
rmlMatrix* rmlAllocateDotMatrix(rmlMatrix* in1, rmlMatrix* in2) {
	if (in1->col != in2->row) {
		printf("RML_ERROR_ALLOCATEDOTMATRIX: in1->col != in2->row.\n");
		return NULL;
	}
	rmlMatrix* out = rmlAllocateMatrix(in1->row, in2->col);
	for (unsigned int i = 0; i < in1->row; i++) {
		for (unsigned int j = 0; j < in2->col; j++) {
			for (unsigned int k = 0; k < in1->col; k++) {
				out->val[i][j] += in1->val[i][k] * in2->val[k][j];
			}
		}
	}
	return out;
} 

//Modifies matrix pointed to by 'out' to the dot product of input matrices.
//If input is m*n.n*p, output is m*p.
//Not commutative. Advised to do scaling -> rotation -> translation, which is t.(r.s).
//Use this version in render loop.
void rmlModifyDotMatrix(rmlMatrix* in1, rmlMatrix* in2, rmlMatrix* out) {
	if (in1->col != in2->row) {
		printf("RML_ERROR_MODIFYDOTMATRIX: in1->col != in2->row.\n");
		return;
	}
	if (out->row != in1->row || out->col != in2->col) {
		printf("RML_ERROR_MODIFYDOTMATRIX: out->row != in1->row || out->col != in2->col.\n");
		return;
	}
	rmlMatrix* holder = rmlAllocateMatrix(out->row, out->col);
	for (unsigned int i = 0; i < in1->row; i++) {
		for (unsigned int j = 0; j < in2->col; j++) {
			for (unsigned int k = 0; k < in1->col; k++) {
				holder->val[i][j] += in1->val[i][k] * in2->val[k][j];
			}
		}
	}
	rmlCopyMatrix(holder, out);
	rmlFreeMatrix(holder);
	//out->val[i][j] = in1->val[i][k] * in2->val[k][j];
} 

//Returns dot product of two vectors.
//Commutative.
float rmlDotVector(rmlVector* in1, rmlVector* in2) {
	if (in1->size != in2->size) {
		printf("RML_ERROR_DOTVECTOR: in1->size != in2->size.\n");
		return NAN;
	}
	float out = 0.0;
	for (unsigned int i = 0; i < in1->size; i++) {
		out += in1->val[i] * in2->val[i];
	}
	return out;
} 

//Returns pointer to vector that is dot product of input matrix and vector.
//Dynamically allocates. User must free with rmlFreeMatrix.
rmlVector* rmlDotMatrixVector(rmlMatrix* mat, rmlVector* vec) {
	if (mat->col != vec->size) {
		printf("RML_ERROR_DOTMATRIXVECTOR: mat->col != vec->size.\n");
		return NULL;
	}
	rmlVector* out = rmlAllocateVector(mat->row);
	for (unsigned int i = 0; i < mat->row; i++) {
		for (unsigned int j = 0; j < mat->col; j++) {
			out->val[i] += mat->val[i][j] * vec->val[j];
		}
	}
	return out;
}

//Copies values of matrix 'in' to matrix 'out' (get same values, different address).
void rmlCopyMatrix(rmlMatrix* in, rmlMatrix* out) {
	if (in->col != out->col || in->row != out->row) {
		printf("RML_ERROR_COPYMATRIX: in->col != out->col || in->row != out->row.\n");
		return;
	}
	for (unsigned int i = 0; i < in->row; i++) {
		for (unsigned int j = 0; j < in->col; j++) {
			out->val[i][j] = in->val[i][j];
		}
	}
} 

void rmlFillMatrix(float scalar, rmlMatrix* mat) {
	if (mat != NULL) {
		for (unsigned int i = 0; i < mat->row; i++) {
			for (unsigned int j = 0; j < mat->col; j++) {
				mat->val[i][j] = scalar;
			}
		}
	} else printf("RML_ERROR_FILLMATRIX: passed NULL.\n");
}

void rmlFillVector(float scalar, rmlVector* vec) {
	if (vec != NULL) {
		for (unsigned int i = 0; i < vec->size; i++) {
			vec->val[i] = scalar;
		}
	} else printf("RML_ERROR_FILLVECTOR: passed NULL.\n");
}

//Multiplies every member of matrix by scalar.
//Uniform scale.
void rmlScalarByMatrix(float scalar, rmlMatrix* mat) {
	if (mat != NULL) {
		for (unsigned int i = 0; i < mat->row; i++) {
			for (unsigned int j = 0; j < mat->col; j++) {
				mat->val[i][j] = scalar * mat->val[i][j];
			}
		}
	} else printf("RML_ERROR_SCALARBYMATRIX: passed NULL.\n");
}

//Multiplies every member of vector by scalar.
//Uniform scale.
void rmlScalarByVector(float scalar, rmlVector* vec) {
	if (vec != NULL) {
		for (unsigned int i = 0; i < vec->size; i++) {
			vec->val[i] = scalar * vec->val[i];
		}
	} else printf("RML_ERROR_SCALARBYVECTOR: passed NULL.\n");
}

//For 'op', 1 is plus and 0 is minus.
rmlVector* rmlAddVector(rmlVector* in1, bool op ,rmlVector* in2) {
	if (in1->size != in2->size) {
		printf("RML_ERROR_ADDVECTOR: in1->size != in2->size.\n");
		return NULL;
	}
	unsigned int sign = op == 1 ? 1 : -1;
	rmlVector* out = rmlAllocateVector(in1->size);
	for (unsigned int i = 0; i < out->size; i++) {
		out->val[i] += in1->val[i] + sign * in2->val[i];
	}
	return out;
}

//Returns pointer to vector that is cross product of input vectors.
//Only defined for size = 3.
//Anticommutative, so a x b = - b x a.
//Dynamically allocates. User must free with rmlFreeVector.
rmlVector* rmlCrossVector(rmlVector* in1, rmlVector* in2) {
	if (in1->size != in2->size || in1->size != 3) {
		printf("RML_ERROR_CROSSVECTOR: in1->size != in2->size || in1->size != 3.\n");
		return NULL;
	}
	rmlVector* out = rmlAllocateVector(3);
	out->val[0] = in1->val[1] * in2->val[2] - in1->val[2] * in2->val[1];		
	out->val[1] = in1->val[2] * in2->val[0] - in1->val[0] * in2->val[2];		
	out->val[2] = in1->val[0] * in2->val[1] - in1->val[1] * in2->val[0];		
	return out;
}

void rmlNormaliseVector(rmlVector* vec) {
	if (vec != NULL) {
		float length = rmlLengthOfVector(vec);
		if (length == 0.0) {
			printf("RML_ERROR_NORMALVECTOR: length == 0.0.\n");
			return;
		}
		vec->val[0] = vec->val[0] / length;
		vec->val[1] = vec->val[1] / length;
		vec->val[2] = vec->val[2] / length;
	} else printf("RML_ERROR_NORMALVECTOR: passed NULL.\n");
}

float rmlLengthOfVector(rmlVector* vec) {
	if (vec != NULL) {
		float length = 0.0;
		for (unsigned int i = 0; i < vec->size; i++) {
			length += vec->val[i]*vec->val[i];
		}
		return sqrt(length);
	} else printf("RML_ERROR_LENGTHOFVECTOR: passed NULL.\n");
	return NAN;
}

//Returns pointer to size*size identity matrix.
//Dynamically allocates. User must free with rmlFreeMatrix.
rmlMatrix* rmlIdentityMatrix(unsigned int size) {
	rmlMatrix* mat = rmlAllocateMatrix(size, size);
	for (unsigned int i = 0; i < mat->row; i++) {
		for (unsigned int j = 0; j < mat->col; j++) {
			if (i == j) {
				mat->val[i][j] = 1.0;
			} else mat->val[i][j] = 0.0;
		}
	}
	return mat;
}

//Returns pointer to 3*3 or 4*4 scaling matrix specified by 'vec'.
//Not uniform scale.
//Dynamically allocates. User must free with rmlFreeMatrix.
rmlMatrix* rmlAllocateScalingMatrix(rmlVector* vec) {	
	if (!(vec->size == 2 || vec->size == 3)) {
		printf("RML_ERROR_ALLOCATESCALINGMATRIX: !(vec->size == 2 || vec->size == 3).\n");
		return NULL;
	}
	rmlMatrix* mat = rmlAllocateMatrix(vec->size + 1, vec->size + 1);
	for (unsigned int i = 0; i < mat->row; i++) {
		for (unsigned int j = 0; j < mat->col; j++) {
			if (i == j && i != vec->size) {
				mat->val[i][j] = vec->val[i];
			} else if (i == j && i == vec->size) mat->val[i][j] = 1.0;
		}
	}
	return mat;
}

//Modifies matrix pointed to by 'mat' to scaling matrix specified by 'vec'.
//Not uniform scale.
//Use this version in render loop.
void rmlModifyScalingMatrix(rmlVector* vec, rmlMatrix* mat) {	
	if (!(vec->size == 2 || vec->size  == 3)) {
		printf("RML_ERROR_MODIFYSCALINGMATRIX: !(vec->size == 2 || vec->size  == 3).\n");
		return;
	}
	if (mat->row != mat->col || mat->col != vec->size + 1) {
		printf("RML_ERROR_MODIFYSCALINGMATRIX: mat->row != mat->col || mat->col != vec->size + 1.\n");
		return;
	}
	for (unsigned int i = 0; i < mat->row; i++) {
		for (unsigned int j = 0; j < mat->col; j++) {
			if (i == j && i != vec->size) {
				mat->val[i][j] = vec->val[i];
			} else if (i == j && i == vec->size) mat->val[i][j] = 1.0;
			else mat->val[i][j] = 0.0;
		}
	}
}

//Returns pointer to 3*3 or 4*4 translation matrix specified by 'vec'.
//Dynamically allocates. User must free with rmlFreeMatrix.
rmlMatrix* rmlAllocateTranslationMatrix(rmlVector* vec) {
	if (!(vec->size == 2 || vec->size == 3)) {
		printf("RML_ERROR_ALLOCATETRANSLATIONMATRIX: !(vec->size == 2 || vec->size == 3).\n");
		return NULL;
	}
	rmlMatrix* mat = rmlIdentityMatrix(vec->size + 1);
	mat->val[0][mat->col - 1] = vec->val[0];
	mat->val[1][mat->col - 1] = vec->val[1];
	if (vec->size == 3) mat->val[2][mat->col - 1] = vec->val[2];
	return mat;
}

//Modifies matrix pointed to by 'mat' to translation matrix specified by 'vec'.
//Use this version in render loop.
void rmlModifyTranslationMatrix(rmlVector* vec, rmlMatrix* mat) {
	if (!(vec->size == 2 || vec->size == 3)) {
		printf("RML_ERROR_MODIFYTRANSLATIONMATRIX: !(vec->size == 2 || vec->size == 3).\n");
		return;
	}
	if (mat->row != mat->col || mat->col != vec->size + 1) {
		printf("RML_ERROR_MODIFYTRANSLATIONMATRIX: mat->row != mat->col || mat->col != vec->size + 1.\n");
		return;
	}
	for (unsigned int i = 0; i < mat->row; i++) {
		for (unsigned int j = 0; j < mat->col; j++) {
			if (i == j) {
				mat->val[i][j] = 1.0;
			} else mat->val[i][j] = 0.0;
		}
	}
	mat->val[0][mat->col - 1] = vec->val[0];
	mat->val[1][mat->col - 1] = vec->val[1];
	if (vec->size == 3) mat->val[2][mat->col - 1] = vec->val[2];
}

//Returns pointer to 3*3 or 4*4 rotation matrix specified by rotation axis 'vec' and angle 'rad' (ccw +ve radians).
//Can result in gimbal lock.
//Dynamically allocates. User must free with rmlFreeMatrix.
rmlMatrix* rmlAllocateRotationMatrix(rmlVector* vec, double rad) {
	if (!(vec->size == 2 || vec->size == 3)) {
		printf("RML_ERROR_ALLOCATEROTATIONMATRIX: !(vec->size == 2 || vec->size == 3).\n");
		return NULL;
	}
	rmlMatrix* mat = rmlAllocateMatrix(vec->size + 1, vec->size + 1);
	mat->val[mat->row - 1][mat->col - 1] = 1.0;
	if (vec->size == 2) {
		mat->val[0][0] = cos(rad);	
		mat->val[0][1] = sin(rad);	
		mat->val[1][0] = -sin(rad);	
		mat->val[1][1] = cos(rad);	
	} else {
		mat->val[0][0] = cos(rad) + vec->val[0] * vec->val[0] * (1 - cos(rad));
		mat->val[0][1] = vec->val[0] * vec->val[1] * (1 - cos(rad)) - vec->val[2] * sin(rad);	
		mat->val[0][2] = vec->val[0] * vec->val[2] * (1 - cos(rad)) + vec->val[1] * sin(rad);
		mat->val[1][0] = vec->val[1] * vec->val[0] * (1 - cos(rad)) + vec->val[2] * sin(rad);
		mat->val[1][1] = cos(rad) + vec->val[1] * vec->val[1] * (1 - cos(rad));
		mat->val[1][2] = vec->val[1] * vec->val[2] * (1 - cos(rad)) - vec->val[0] * sin(rad);
		mat->val[2][0] = vec->val[2] * vec->val[0] * (1 - cos(rad)) - vec->val[1] * sin(rad);
		mat->val[2][1] = vec->val[2] * vec->val[1] * (1 - cos(rad)) + vec->val[0] * sin(rad);
		mat->val[2][2] = cos(rad) + vec->val[2] * vec->val[2] * (1 - cos(rad));
	}	
	return mat;
}

//Modifies matrix pointed to by 'mat' to rotation matrix specified by rotation axis 'vec' and angle 'rad' (ccw +ve radians).
//Can result in gimbal lock.
//Use this version in render loop.
void rmlModifyRotationMatrix(rmlVector* vec, double rad, rmlMatrix* mat) {
	if (!(vec->size == 2 || vec->size == 3)) {
		printf("RML_ERROR_MODIFYROTATIONMATRIX: !(vec->size == 2 || vec->size == 3).\n");
		return;
	}
	if (mat->row != mat->col || mat->col != vec->size + 1) {
		printf("RML_ERROR_MODIFYROTATIONMATRIX: mat->row != mat->col || mat->col != vec->size + 1.\n");
		return;
	}
	mat->val[mat->row - 1][mat->col - 1] = 1.0;
	for (unsigned int i = 0; i < mat->row; i++) {
		for (unsigned int j = 0; j < mat->col; j++) {
			if (i == j && i != vec->size) {
				mat->val[i][j] = 0.0;
			}
		}
	}
	if (vec->size == 2) {
		mat->val[0][0] = cos(rad);	
		mat->val[0][1] = sin(rad);	
		mat->val[1][0] = -sin(rad);	
		mat->val[1][1] = cos(rad);	
	} else {
		mat->val[0][0] = cos(rad) + vec->val[0] * vec->val[0] * (1 - cos(rad));
		mat->val[0][1] = vec->val[0] * vec->val[1] * (1 - cos(rad)) - vec->val[2] * sin(rad);	
		mat->val[0][2] = vec->val[0] * vec->val[2] * (1 - cos(rad)) + vec->val[1] * sin(rad);
		mat->val[1][0] = vec->val[1] * vec->val[0] * (1 - cos(rad)) + vec->val[2] * sin(rad);
		mat->val[1][1] = cos(rad) + vec->val[1] * vec->val[1] * (1 - cos(rad));
		mat->val[1][2] = vec->val[1] * vec->val[2] * (1 - cos(rad)) - vec->val[0] * sin(rad);
		mat->val[2][0] = vec->val[2] * vec->val[0] * (1 - cos(rad)) - vec->val[1] * sin(rad);
		mat->val[2][1] = vec->val[2] * vec->val[1] * (1 - cos(rad)) + vec->val[0] * sin(rad);
		mat->val[2][2] = cos(rad) + vec->val[2] * vec->val[2] * (1 - cos(rad));
	}	
}

/*
//Local space -> world space.
//Import model in local space coords.
void rmlApplyModelMatrix(rmlMatrix* mat) {
	
}

//World space -> view space.
void rmlViewMatrix(float **mat, unsigned int row, unsigned int col, float x, float y, float z) {

}
*/
/*
rmlMatrix* rmlAllocateProjectionMatrix(float angleOfView, float imageAspectRatio, float n, float f) {
	float scale = tan(angleOfView * 0.5 * M_PI / 180) * n; 
	float r = imageAspectRatio * scale; float l = -r; 
	float t = scale; float b = -t; 	
	rmlMatrix* mat = rmlAllocateMatrix(4, 4);
	mat->val[0][0] = 2 * n / (r - l);
	mat->val[1][1] = 2 * n / (t - b);
	mat->val[2][0] = (r + l) / (r - l);
	mat->val[2][1] = (t + b) / (t - b);
	mat->val[2][2] = -(f + n) / (f - n);
	mat->val[2][3] = -1;
	mat->val[3][2] = -2 * f * n / (f - n);
	return mat;
}

void rmlModifyProjectionMatrix(float angleOfView, float imageAspectRatio, float n,float f, rmlMatrix* mat) {
	float scale = tan(angleOfView * 0.5 * M_PI / 180) * n;
	float r = imageAspectRatio * scale; float l = -r;
	float t = scale; float b = -t;
	rmlFillMatrix(0.0, mat);
	mat->val[0][0] = 2 * n / (r - l);
	mat->val[1][1] = 2 * n / (t - b);
	mat->val[2][0] = (r + l) / (r - l);
	mat->val[2][1] = (t + b) / (t - b);
	mat->val[2][2] = -(f + n) / (f - n);
	mat->val[2][3] = -1;
	mat->val[3][2] = -2 * f * n / (f - n);
}
*/

//View space -> clip space. Only defined for 4*4.
//Run viewport transform after to get screen space.
rmlMatrix* rmlAllocateProjectionMatrix(unsigned int h, unsigned int w, double fov, float zFar, float zNear) {
//	if (h == 0 || w == 0 || zNear == 0 || zNear >= zFar) {
//		printf("RML_ERROR_ALLOCATEPROJECTIONMATRIX: h == 0 || w == 0 || zNear == 0 || zNear >= zFar.\n");
//		return NULL;
//	}
	rmlMatrix* mat = rmlAllocateMatrix(4, 4);
	mat->val[0][0] = ((float)h / (float)w) / tan(fov / 2);
	mat->val[1][1] = 1 / tan(fov / 2);
	mat->val[2][2] = zFar / (zFar - zNear);
	mat->val[3][2] = (-zFar * zNear) / (zFar - zNear);
	mat->val[2][3] = 1.0;
	return mat;
}

void rmlModifyProjectionMatrix(unsigned int h, unsigned int w, double fov, float zFar, float zNear, rmlMatrix* mat) {
//	if (h == 0 || w == 0 || zNear == 0 || zNear >= zFar) {
//		printf("RML_ERROR_ALLOCATEPROJECTIONMATRIX: h == 0 || w == 0 || zNear == 0 || zNear >= zFar.\n");
//		return NULL;
//	}
	rmlFillMatrix(0.0, mat);
	mat->val[0][0] = ((float)h / (float)w) / tan(fov / 2);
	mat->val[1][1] = 1 / tan(fov / 2);
	mat->val[2][2] = zFar / (zFar - zNear);
	mat->val[3][2] = (-zFar * zNear) / (zFar - zNear);
	mat->val[2][3] = 1.0;
}

/*
//Modifies matrix pounsigned inted to by 'mat' from row-major to column-major or vice versa.
void rmlSwitchMajor(rmlMatrix* mat) {
}
*/
