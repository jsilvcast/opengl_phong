//
// Created by v on 7/06/23.
//

#ifndef GL_TEST_TETRAHEDRON_H
#define GL_TEST_TETRAHEDRON_H

#include <GL/glew.h>

class Tetrahedron {
private:
    // Tetrahedron to be rendered
    //
    //                   3
    //       7
    // bottom
    // left
    // far ---> 1
    //                5
    //

    GLfloat vertex[36] =  {
            -0.25f, -0.25f, -0.25f, // 1
            -0.25f,  0.25f,  0.25f, // 7
            0.25f, -0.25f,  0.25f, // 5

            -0.25f,  0.25f,  0.25f, // 7
            0.25f, -0.25f,  0.25f, // 5
            0.25f,  0.25f, -0.25f, // 3

            -0.25f,  0.25f,  0.25f, // 7
            0.25f,  0.25f, -0.25f, // 3
            -0.25f, -0.25f, -0.25f, // 1

            0.25f,  0.25f, -0.25f, // 3
            -0.25f, -0.25f, -0.25f, // 1
            0.25f, -0.25f,  0.25f, // 5
    };

    GLfloat vertex_normal[36] = {
            -1.f, -1.f,  1.f, // 1
            -1.f, -1.f,  1.f, // 7
            -1.f, -1.f,  1.f, // 5

            1.f, 1.f,  1.f, // 7
            1.f, 1.f,  1.f, // 5
            1.f, 1.f,  1.f, // 3

            -1.f, 1.f,  -1.f, // 7
            -1.f, 1.f,  -1.f, // 3
            -1.f, 1.f,  -1.f, // 1

            1.f, -1.f,  -1.f, // 3
            1.f, -1.f,  -1.f, // 1
            1.f, -1.f,  -1.f, // 5


    };

public:
    Tetrahedron() = default;
    GLfloat* getVertices() {
        return vertex;
    }

    GLfloat* getNormals() {
        return vertex_normal;
    }
};

#endif //GL_TEST_TETRAHEDRON_H