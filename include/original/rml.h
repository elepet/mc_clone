//Renderer Matrix Library

void rmlPrint(float mat[size][size]);
void rmlFill(float scalar, float mat[size][size]);
void rmlDot(float in1[size][size], float in2[size][size], float out[size][size]);
void rmlScaleMat(float vec[size - 1], float out[size][size]);
void rmlRotateMat(float vec[size - 1], double rad, float out[size][size]);
void rmlTranslateMat(float vec[size - 1], float out[size][size]);
void rmlProjectMat(unsigned int h, unsigned int w, double fov, float zFar, float zNear, float out[size][size]);
void rmlNormaliseVec(float vec[size - 1]);
float rmlLengthOfVec(float vec[size - 1]);
