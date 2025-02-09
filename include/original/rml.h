//Renderer Matrix Library

void rmlPrint(float mat[size][size]);
void rmlFill(float scalar, float mat[size][size]);
void rmlDot(float in1[size][size], float in2[size][size], float out[size][size]);
void rmlScale(float vec[size - 1], float out[size][size]);
void rmlRotate(float vec[size - 1], double rad, float out[size][size]);
void rmlTranslate(float vec[size - 1], float out[size][size]);
void rmlProject(float angleOfView, int w, int h, float n, float f, float M[size][size]);
void rmlNormaliseVec(float vec[size - 1]);
float rmlLengthOfVec(float vec[size - 1]);
