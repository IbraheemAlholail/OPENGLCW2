#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/torus.h"
#include "helper/teapot.h"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/frustum.h"
#include "GLFW/glfw3.h"

class SceneBasic_Uniform : public Scene
{
public:
    Plane plane;
    float tPrev,angle,time,deltaT,rotSpeed,spinAngle;
    int nParticles;
    std::unique_ptr<ObjMesh> meshBowl, meshFish;


    GLSLProgram shadowProg, solidProg, PBRProg;
    GLuint shadowFBO, pass1Index, pass2Index;
    int shadowMapWidth, shadowMapHeight;
    glm::mat4 lightPV, shadowBias;
    Frustum lightFrustum;
    glm::vec4 lightpos;

    void setmatrices();
    void compile();
    void drawScene();
    void setupFBO();
    void fishJump();


public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    void processInput(GLFWwindow *window);
    
};

#endif // SCENEBASIC_UNIFORM_H
