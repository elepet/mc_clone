// Cube vertices.

#ifndef VERTICES_H
#define VERTICES_H

float vertices[] = { // CCW winding.
    // x     y     z     u     v
    // Back face
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Bottom-left
    1.0f, 1.0f, 0.0f,  1.0f, 1.0f, // Top-right
    1.0f, 0.0f, 0.0f,  1.0f, 0.0f, // Bottom-right
    1.0f, 1.0f, 0.0f,  1.0f, 1.0f, // Top-right
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f, // Bottom-left
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Top-left

    // Front face
    0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Bottom-left
    1.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Bottom-right
    1.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Top-right
    1.0f, 1.0f, 1.0f,  1.0f, 1.0f, // Top-right
    0.0f, 1.0f, 1.0f,  0.0f, 1.0f, // Top-left
    0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Bottom-left

    // Left face
    0.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Top-right
    0.0f, 1.0f, 0.0f,  1.0f, 1.0f, // Top-left
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, // Bottom-left
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, // Bottom-left
    0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Bottom-right
    0.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Top-right

    // Right face
    1.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Top-left
    1.0f, 0.0f, 0.0f,  0.0f, 1.0f, // Bottom-right
    1.0f, 1.0f, 0.0f,  1.0f, 1.0f, // Top-right
    1.0f, 0.0f, 0.0f,  0.0f, 1.0f, // Bottom-right
    1.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Top-left
    1.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Bottom-left

    // Bottom face
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, // Top-right
    1.0f, 0.0f, 0.0f,  1.0f, 1.0f, // Top-left
    1.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Bottom-left
    1.0f, 0.0f, 1.0f,  1.0f, 0.0f, // Bottom-left
    0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // Bottom-right
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, // Top-right

    // Top face
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Top-left
    1.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Bottom-right
    1.0f, 1.0f, 0.0f,  1.0f, 1.0f, // Top-right
    1.0f, 1.0f, 1.0f,  1.0f, 0.0f, // Bottom-right
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, // Top-left
    0.0f, 1.0f, 1.0f,  0.0f, 0.0f  // Bottom-left
};
#endif
