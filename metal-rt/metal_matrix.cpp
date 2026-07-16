// metal_matrix.cpp - Metal Matrix Operations

extern "C" {

// Matrix operations are typically inlined by the Metal compiler
// These are fallback implementations for host-side testing

// 2x2 matrix multiply
void ___metal_matrix_multiply_2x2_float(const float* a, const float* b, float* result) {
    result[0] = a[0]*b[0] + a[2]*b[1];
    result[1] = a[1]*b[0] + a[3]*b[1];
    result[2] = a[0]*b[2] + a[2]*b[3];
    result[3] = a[1]*b[2] + a[3]*b[3];
}

// 3x3 matrix multiply
void ___metal_matrix_multiply_3x3_float(const float* a, const float* b, float* result) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            float sum = 0;
            for (int k = 0; k < 3; k++) sum += a[k*3+i] * b[j*3+k];
            result[j*3+i] = sum;
        }
}

// 4x4 matrix multiply
void ___metal_matrix_multiply_4x4_float(const float* a, const float* b, float* result) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            float sum = 0;
            for (int k = 0; k < 4; k++) sum += a[k*4+i] * b[j*4+k];
            result[j*4+i] = sum;
        }
}

// Matrix determinant
float ___metal_determinant_2x2(const float* m) { return m[0]*m[3] - m[1]*m[2]; }
float ___metal_determinant_3x3(const float* m) {
    return m[0]*(m[4]*m[8]-m[5]*m[7]) - m[1]*(m[3]*m[8]-m[5]*m[6]) + m[2]*(m[3]*m[7]-m[4]*m[6]);
}
float ___metal_determinant_4x4(const float* m) {
    float s0 = m[0]*m[5] - m[4]*m[1]; float s1 = m[0]*m[6] - m[4]*m[2];
    float s2 = m[0]*m[7] - m[4]*m[3]; float s3 = m[1]*m[6] - m[5]*m[2];
    float s4 = m[1]*m[7] - m[5]*m[3]; float s5 = m[2]*m[7] - m[6]*m[3];
    float c5 = m[10]*m[15] - m[14]*m[11]; float c4 = m[9]*m[15] - m[13]*m[11];
    float c3 = m[9]*m[14] - m[13]*m[10]; float c2 = m[8]*m[15] - m[12]*m[11];
    float c1 = m[8]*m[14] - m[12]*m[10]; float c0 = m[8]*m[13] - m[12]*m[9];
    return s0*c5 - s1*c4 + s2*c3 + s3*c2 - s4*c1 + s5*c0;
}

// Matrix transpose
void ___metal_transpose_2x2(const float* m, float* result) {
    result[0]=m[0]; result[1]=m[2]; result[2]=m[1]; result[3]=m[3];
}
void ___metal_transpose_3x3(const float* m, float* result) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            result[i*3+j] = m[j*3+i];
}
void ___metal_transpose_4x4(const float* m, float* result) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            result[i*4+j] = m[j*4+i];
}

// Matrix inverse
void ___metal_inverse_2x2(const float* m, float* result) {
    float det = ___metal_determinant_2x2(m);
    if (det == 0) { for (int i=0;i<4;i++) result[i]=0; return; }
    float inv = 1.0f / det;
    result[0] =  m[3]*inv; result[1] = -m[1]*inv;
    result[2] = -m[2]*inv; result[3] =  m[0]*inv;
}

} // extern C
