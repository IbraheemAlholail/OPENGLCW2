#include "scenebasic_uniform.h"

#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;

vec3 fishPos = vec3(0.0f, 0.0f, 0.0f);
vec3 fishStartPos = fishPos;

float fishSpin = 0.0f;

SceneBasic_Uniform::SceneBasic_Uniform() :
    tPrev(0), 
    rotSpeed(0.2f),
    shadowMapHeight(512), shadowMapWidth(512),
    plane(50.0f,50.0f,1,1)
    {
    meshBowl = ObjMesh::load("media/bowl.obj", true);
    meshFish = ObjMesh::load("media/fish.obj", true);
}

void SceneBasic_Uniform::initScene()
{
    compile();
    glClearColor(0.5f,0.5f,0.5f,1.0f);
    glEnable(GL_DEPTH_TEST);
    angle = glm::quarter_pi<float>();
    spinAngle = glm::radians(70.0f);

    view = glm::lookAt(vec3(0.0f, 5.0f, 15.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

    //model = mat4(1.0f);
    setupFBO();
    GLuint programHandle = shadowProg.getHandle();
    pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "recordDepth");
    pass2Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "shadeWithShadow");
    shadowBias = mat4( 
        vec4(0.5f, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, 0.5f, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 0.5f, 0.0f),
        vec4(0.5f, 0.5f, 0.5f, 1.0f)
    );

    float c = 1.65f;
    vec3 lightPos = vec3(0.0f, c * 5.25f, c * 7.5f);
    lightFrustum.orient(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
    lightFrustum.setPerspective(50.0f, 1.0f, 1.0f, 25.0f);
    lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();
    setupFBO();


    shadowProg.use();
    shadowProg.setUniform("Light.Intensity", vec3(1.0f));
    shadowProg.setUniform("Light.La", vec3(1.0f));
    shadowProg.setUniform("Light.L", vec3(1.0f));
    shadowProg.setUniform("ShadowMap", 0);

    PBRProg.use();
    PBRProg.setUniform("Light[0].L", vec3(45.0f));
    PBRProg.setUniform("Light[0].Position", view * vec4(5.0f,5.0f,5.0f,1.0f));
    PBRProg.setUniform("Light[1].L", vec3(0.3f));
    PBRProg.setUniform("Light[1].Position", view * vec4(0.0f,0.15f,1.0f,0));
    PBRProg.setUniform("Light[2].L", vec3(45.0f));
    PBRProg.setUniform("Light[2].Position", view * vec4(7.0f,3.0f,7.0f,1.0f));

    shadowProg.use();
}

void SceneBasic_Uniform::compile()
{
	try {
        shadowProg.compileShader("shader/basic_uniform.vert");
        shadowProg.compileShader("shader/basic_uniform.frag");
        shadowProg.link();
        shadowProg.use();

        solidProg.compileShader("shader/solid.vert");
        solidProg.compileShader("shader/solid.frag");
        solidProg.link();

        PBRProg.compileShader("shader/PBR.vert");
		PBRProg.compileShader("shader/PBR.frag");
        PBRProg.link();

	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update(float t)
{
    float deltaT = t - tPrev;
    if (tPrev == 0.0f) deltaT = 0.0f;
    tPrev = t;

    //process the fish's movement
    GLFWwindow* window = glfwGetCurrentContext();
    processInput(window);

}




void SceneBasic_Uniform::render()
{
    shadowProg.use();

    // Pass 1 (shadow map generation)
    view = lightFrustum.getViewMatrix();
    projection = lightFrustum.getProjectionMatrix();
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.5f, 10.0f);
    drawScene();
    glCullFace(GL_BACK);
    glFlush();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render pass
    shadowProg.setUniform("Pass2", 1);
    float c = 2.0f;
    vec3 cameraPos = vec3(c * 11.5f * cos(spinAngle), c * 7.0f, c * 11.5f * sin(spinAngle));
    view = glm::lookAt(cameraPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    shadowProg.setUniform("Light.Position", view * vec4(lightFrustum.getOrigin(), 1.0f));
    projection = glm::perspective(glm::radians(50.0f), (float)width / height, 0.1f, 100.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);
    PBRProg.setUniform("Light[0].Position", view * vec4(5.0f, 5.0f, 5.0f, 1.0f));

    drawScene();
    
    solidProg.use();
    solidProg.setUniform("Color", vec4(1.0f, 0.0f, 0.0f, 1.0f));
    mat4 mv = view * lightFrustum.getInverseViewMatrix();
    solidProg.setUniform("MVP", projection * mv);
    lightFrustum.render();
}


void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    
}

void SceneBasic_Uniform::setmatrices()
{
    mat4 mv = view * model;
    shadowProg.setUniform("ModelViewMatrix", mv);
    shadowProg.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    shadowProg.setUniform("MVP", projection * mv);
    shadowProg.setUniform("ShadowMatrix", lightPV * model);

    PBRProg.setUniform("ModelViewMatrix", mv);
    PBRProg.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    PBRProg.setUniform("MVP", projection * mv);
}

void SceneBasic_Uniform::drawScene()
{
    vec3 color = vec3(0.1f, 0.25f, 0.7f);

    //render the plane
    shadowProg.setUniform("Material.Kd", vec3(0.25f,0.25f,0.25f));
    shadowProg.setUniform("Material.Ka", vec3(0.05f, 0.05f, 0.05f));
    shadowProg.setUniform("Material.Ks", vec3(0.0f, 0.0f, 0.0f));
    shadowProg.setUniform("Material.Shininess", 1.0f);
    model = mat4(1.0f);
    setmatrices();
    plane.render();

    model = mat4(1.0f);
    model = glm::translate(model, vec3(-5.0f, 5.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
    setmatrices();
    plane.render();

    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 5.0f, -5.0f));
    model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    setmatrices();
    plane.render();

    //render the fish 
    shadowProg.setUniform("Material.Kd", vec3(1.0f, 0.0f, 0.0f));
    shadowProg.setUniform("Material.Ka", vec3(0.05f, 0.05f, 0.05f));
    shadowProg.setUniform("Material.Ks", vec3(0.0f, 0.0f, 0.0f));
    shadowProg.setUniform("Material.Shininess", 1.0f);
    model = mat4(1.0f);
    model = glm::translate(model, fishPos);
    model = glm::rotate(model, fishSpin, vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, vec3(0.5f));


    setmatrices();
    meshFish->render();

    PBRProg.use();

    PBRProg.setUniform("Material.Rough", 0.43f);
    PBRProg.setUniform("Material.Metal", 1);
    PBRProg.setUniform("Material.Color", color);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 2.0f, 0.0f));
    model = glm::scale(model, vec3(vec3(2.0f)));
    model = glm::rotate(model, glm::radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    setmatrices();
    meshBowl->render();
    shadowProg.use();
}

void SceneBasic_Uniform::setupFBO() {

    GLfloat border[] = {1.0f, 0.0f, 0.0f, 0.0f};
    GLuint depthTex;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTex);

    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0); 

    GLenum drawBuffers[] = {GL_NONE};
    glDrawBuffers(1, drawBuffers);
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer is complete.\n");
    }
    else {
		printf("Framebuffer is not complete.\n");
	}
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		fishPos.z -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		fishPos.z += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		fishPos.x -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		fishPos.x += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        fishPos.y = 1.5f;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        fishPos.y = 0.1f;

    //spin the fish with the arrow keys
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		fishSpin += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        fishSpin -= 0.1f;

    //spin the camera with the arrow keys, with a minimum and maximum angle

    float minAngle = glm::radians(0.0f);
    float maxAngle = glm::radians(90.0f);

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		spinAngle += 0.01f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		spinAngle -= 0.01f;

    if (spinAngle < minAngle)
		spinAngle = minAngle;
	if (spinAngle > maxAngle)
		spinAngle = maxAngle;

}





