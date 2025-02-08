//Renderer Matrix Library

//Matrices are row-major. To use with OpenGL, in render loop:
//unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
//glUniformMatrix4fv(transformLoc, 1, GL_TRUE, transformMat->val[0]);
typedef struct {
	unsigned int row;
	unsigned int col;
	float** val;
} rmlMatrix;

typedef struct {
	unsigned int size;
	float* val;
} rmlVector;

rmlMatrix* rmlAllocateMatrix(unsigned int row, unsigned int col);
void rmlFreeMatrix(rmlMatrix* mat);
void rmlPrintMatrix(rmlMatrix* mat);
rmlVector* rmlAllocateVector(unsigned int size);
void rmlFreeVector(rmlVector* vec);
void rmlPrintVector(rmlVector* vec);
rmlMatrix* rmlAllocateDotMatrix(rmlMatrix* in1, rmlMatrix* in2);
void rmlModifyDotMatrix(rmlMatrix* in1, rmlMatrix* in2, rmlMatrix* out);
float rmlDotVector(rmlVector* in1, rmlVector* in2);
rmlVector* rmlDotMatrixVector(rmlMatrix* mat, rmlVector* vec);
void rmlCopyMatrix(rmlMatrix* in, rmlMatrix* out);
void rmlFillMatrix(float scalar, rmlMatrix* mat);
void rmlFillVector(float scalar, rmlVector* vec);
void rmlScalarByMatrix(float scalar, rmlMatrix* mat);
void rmlScalarByVector(float scalar, rmlVector* vec);
rmlVector* rmlAddVector(rmlVector* in1, bool op ,rmlVector* in2);
rmlVector* rmlCrossVector(rmlVector* in1, rmlVector* in2);
void rmlNormaliseVector(rmlVector* vec);
float rmlLengthOfVector(rmlVector* vec);
rmlMatrix* rmlIdentityMatrix(unsigned int size);
rmlMatrix* rmlAllocateScalingMatrix(rmlVector* vec);
void rmlModifyScalingMatrix(rmlVector* vec, rmlMatrix* mat);
rmlMatrix* rmlAllocateTranslationMatrix(rmlVector* vec);
void rmlModifyTranslationMatrix(rmlVector* vec, rmlMatrix* mat);
rmlMatrix* rmlAllocateRotationMatrix(rmlVector* vec, double rad);
void rmlModifyRotationMatrix(rmlVector* vec, double rad, rmlMatrix* mat);
rmlMatrix* rmlAllocateProjectionMatrix(unsigned int h, unsigned int w, double fov, float zFar, float zNear);
void rmlModifyProjectionMatrix(unsigned int h, unsigned int w, double fov, float zFar, float zNear, rmlMatrix* mat);
//rmlMatrix* rmlAllocateProjectionMatrix(float angleOfView, float imageAspectRatio, float n, float f);
//void rmlModifyProjectionMatrix(float angleOfView, float imageAspectRatio, float n,float f, rmlMatrix* mat);
