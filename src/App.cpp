/** \file App.cpp */

#include "App.h"

#define FONTSTASH_IMPLEMENTATION
#include <fontstash.h>
#define GLFONTSTASH_IMPLEMENTATION
#include <glfontstash.h>

#include <config/VRDataIndex.h>
#include <main/VRSystem.h>

#include <iostream>
using namespace std;
using namespace glm;

#include <cmath>

// the magnitude of acceleration due to gravity
#define g 1000

App::App(int argc, char** argv) : VRApp(argc, argv) {
    lastTime = VRSystem::getTime();
    ballVel = vec3(0.0, 0.0, 200);
    ballFrame = glm::translate(mat4(1.0), vec3(0, 40, -130));
    ballShadowFrame = ballFrame;
    ballShadowFrame[3][1] = 0;
}

App::~App()
{
    shutdown();
}

void App::onAnalogChange(const VRAnalogEvent &event) {
    if (event.getName() == "FrameStart") {
        lastTime = curFrameTime;
        lastPos = paddleFrame;
        curFrameTime = event.getValue();
        curPos = paddleFrame;

    }

}


void App::onButtonDown(const VRButtonEvent &event) {
    // This routine is called for all Button_Down events.  Check event->getName()
    // to see exactly which button has been pressed down.
}

void App::onButtonUp(const VRButtonEvent &event) {
    // This routine is called for all Button_Up events.  Check event->getName()
    // to see exactly which button has been released.

    //std::cout<<event.getName()<<std::endl;

    if (event.getName() == "KbdSpace_Up") {

      ballVel = vec3(0.0, 0.0, 200);
      ballFrame = glm::translate(mat4(1.0), vec3(0, 40, -130));
      ballShadowFrame = ballFrame;
      ballShadowFrame[3][1] = 0;




        // This is where you can "serve" a new ball from the opponent's side of the net
        // toward you when the spacebar is released. I found that a good initial position for the ball is: (0, 30, -130).
        // And, a good initial velocity is (0, 200, 400).  As usual for this program, all
        // units are in cm.


    }
}

void App::onCursorMove(const VRCursorEvent &event) {
    // This routine is called for all mouse move events. You can get the absolute position
    // or the relative position within the window scaled 0--1.

    //std::cout << "MouseMove: "<< event.getName() << " " << event.getPos()[0] << " " << event.getPos()[1] << std::endl;

    if (event.getName() == "Mouse_Move") {
        vec2 mouseXY(event.getNormalizedPos()[0], event.getNormalizedPos()[1]);


        // This block of code maps the 2D position of the mouse in screen space to a 3D position
        // 20 cm above the ping pong table.  It also rotates the paddle to make the handle move
        // in a cool way.  It also makes sure that the paddle does not cross the net and go onto
        // the opponent's side.
        float xneg1to1 = mouseXY.x * 2.0 - 1.0;
        float y0to1 = mouseXY.y;
        mat4 rotZ = toMat4(angleAxis(glm::sin(-xneg1to1), vec3(0, 0, 1)));


        glm::vec3 lastPaddlePos = glm::column(paddleFrame, 3);
        paddleFrame = glm::translate(mat4(1.0), vec3(xneg1to1 * 100.0, 20.0, glm::max(y0to1 * 137.0 + 20.0, 0.0)));
        paddleShadowFrame = paddleFrame;
        paddleShadowFrame[3][1] = 0;
        paddleFrame*=rotZ;
        vec3 newPos = glm::column(paddleFrame, 3);

        // This is a weighted average.  Update the velocity to be 10% the velocity calculated
        // at the previous frame and 90% the velocity calculated at this frame.
        paddleVel = 0.1f*paddleVel + 0.9f*(newPos - lastPaddlePos);
    }
}

bool App::isOnTable() {
  return ballFrame[3][1] < 2.0 && ballFrame[3][0] > -76.25 && ballFrame[3][0] < 76.25 &&
          ballFrame[3][2] > -137 && ballFrame[3][2] < 137;
}

bool App::hitNet() {
  return ballFrame[3][0] > -91.5 && ballFrame[3][0] < 91.5 && ballFrame[3][1] < 14.25 &&
          ballFrame[3][2] > -2 && ballFrame[3][2] < 2;
}

bool App::hitPaddle() {
  // bool hit = false;
  // if(paddleVel[0] > 0){
  //   return ballFrame[3][0] < paddleFrame[3][0]+8 && ballFrame[3][0] > lastPos[3][0]-8 &&
  //       ballFrame[3][2] < paddleFrame[3][2] && ballFrame[3][2] > lastPos[3][2];
  // } else {
  //   return ballFrame[3][0] > paddleFrame[3][0]-8 && ballFrame[3][0] < lastPos[3][0]+8 &&
  //       ballFrame[3][2] < paddleFrame[3][2] && ballFrame[3][2] > lastPos[3][2];
  // }
  return ballFrame[3][0] - 2 > paddleFrame[3][0] - 8 && ballFrame[3][0] + 2 < paddleFrame[3][0] + 8 &&
          ballFrame[3][2] + 2 > lastPos[3][2] - 1 && ballFrame[3][2] + 2 > curPos[3][2] -1 && ballFrame[3][2] - 2 < paddleFrame[3][2] + 1;
}

void App::onRenderGraphicsContext(const VRGraphicsState &renderState) {
    // This routine is called once per graphics context at the start of the
    // rendering process.  So, this is the place to initialize textures,
    // load models, or do other operations that you only want to do once per
    // frame.

    // Is this the first frame that we are rendering after starting the app?
    if (renderState.isInitialRenderCall()) {

        //For windows, we need to initialize a few more things for it to recognize all of the
        // opengl calls.
#ifndef __APPLE__
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            std::cout << "Error initializing GLEW." << std::endl;
        }
#endif


        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glEnable(GL_MULTISAMPLE);

        // This sets the background color that is used to clear the canvas
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

        // This load shaders from disk, we do it once when the program starts up.
        reloadShaders();


        // Initialize the cylinders that make up the model. We're using unique_ptrs here so they automatically deallocate.
        paddle.reset(new Cylinder(vec3(0, 0, -0.5), vec3(0, 0, 0.5), 8.0, vec4(0.5, 0, 0, 1.0)));
        handle.reset(new Cylinder(vec3(0, -7.5, 0), vec3(0, -16, 0), 1.5, vec4(0.3, 0.4, 0, 1.0)));
        paddleShadow.reset(new Cylinder(vec3(0, 0.2, 0), vec3(0, 0, 0), 8, vec4(0, 0, 0, 0.3)));

        table.reset(new Box(vec3(-76.25, -2, -137), vec3(76.25, 0, 137), vec4(0, 0.5, 0, 1.0)));
        //ball.reset(new Sphere(vec3(0.0, 0.0, 0.0), 2, vec4(1,1,1,1.0)));
        ball.reset(new Sphere(vec3(0, 0, 0), 2, vec4(1,1,1,1.0)));
        ballShadow.reset(new Cylinder(vec3(0, 0.2, 0), vec3(0, 0, 0), 2, vec4(0, 0, 0, 0.3)));


        // This is where you should generate new geometry so it is created once at the start of the program.

    }

    // rdt is the change in time (dt) in seconds since the last frame
    // So, you can slow down the simulation.
    double rdt = curFrameTime - lastTime;
    rdt *= 0.8;

    // Here are a few other values that you may find useful..
    // Radius of the ball = 2cm
    // Radius of the paddle = 8cm
    // Acceleration due to gravity = 981cm/s^2 in the negative Y direction
    // See the diagram in the assignment handout for the dimensions of the ping pong table

    // You should update the ball simulation here
    ballFrame = glm::translate(ballFrame, ballVel * (float) rdt);
    ballShadowFrame = glm::translate(ballShadowFrame, ballVel * (float) rdt);
    ballShadowFrame[3][1] = 0;

    if(isOnTable() && ballVel[1] < 0) {
      ballVel = vec3(ballVel[0], -.96 * ballVel[1], ballVel[2]);
    } else {
      ballVel = ballVel + vec3(0, -1 * g, 0) * (float) rdt;
    }

    if(hitNet()) {
      if(ballVel[2] > 0) {
        ballFrame[3][2] = -2;
      } else {
        ballFrame[3][2] = 2;
      }
      ballVel = vec3(0, 0, 0);
    }

    if(hitPaddle()) {
      ballVel = vec3(ballVel[0]+ paddleVel[0]*4, ballVel[1], -ballVel[2] + paddleVel[2]*2);
    }
}


void App::onRenderGraphicsScene(const VRGraphicsState &renderState) {
    // This routine is called once per eye/camera.  This is the place to actually
    // draw the scene.


    // clear the canvas and other buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Setup the camera with a good initial position and view direction to see the table
    glm::vec3 eye_world = glm::vec3(0, 100, 320);
    glm::mat4 view = glm::lookAt(eye_world, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    //eye_world = glm::vec3(glm::column(glm::inverse(view), 3));

    // Setup the projection matrix so that things are rendered in perspective
    GLfloat windowHeight = renderState.index().getValue("WindowHeight");
    GLfloat windowWidth = renderState.index().getValue("WindowWidth");
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), windowWidth / windowHeight, 0.01f, 500.0f);

    // Setup the model matrix
    glm::mat4 model = glm::mat4(1.0);

    // Update shader variables
    _shader.use();
    _shader.setUniform("view_mat", view);
    _shader.setUniform("projection_mat", projection);
    _shader.setUniform("model_mat", model);
    _shader.setUniform("normal_mat", mat3(transpose(inverse(model))));
    _shader.setUniform("eye_world", eye_world);



    table->draw(_shader, model);

    // Draw the table
    tableLines.reset(new Line(vec3(0, 0, -137), vec3(0, 0, 137), vec3(0,1,0), 0.5, vec4(1,1,1,1))); // center line
    tableLines->draw(_shader, model);

    tableLines.reset(new Line(vec3(-76.25, 0, -137), vec3(-76.25, 0, 137), vec3(0,1,0), 0.5, vec4(1,1,1,1))); // left border
    tableLines->draw(_shader, model);

    tableLines.reset(new Line(vec3(76.25, 0, -137), vec3(76.25, 0, 137), vec3(0,1,0), 0.5, vec4(1,1,1,1))); // right border
    tableLines->draw(_shader, model);

    tableLines.reset(new Line(vec3(-76.25, 0, 137), vec3(76.25, 0, 137), vec3(0,1,0), 0.5, vec4(1,1,1,1))); // front top
    tableLines->draw(_shader, model);

    tableLines.reset(new Line(vec3(-76.25, -2, 137), vec3(76.25, -2, 137), vec3(0,1,0), 0.5, vec4(1,1,1,1)));  // front bottom
    tableLines->draw(_shader, model);

    tableLines.reset(new Line(vec3(-76.25, -2, 137), vec3(-76.25, 0, 137), vec3(0,1,0), 0.5, vec4(1,1,1,1)));  //front left
    tableLines->draw(_shader, model);

    tableLines.reset(new Line(vec3(76.25, -2, 137), vec3(76.25, 0, 137), vec3(0,1,0), 0.5, vec4(1,1,1,1)));  //front right
    tableLines->draw(_shader, model);

    // Draw the net made of grid of lines
    double interval = (91.5 * 2) / 80;
    // verticle lines
    line.reset(new Line(vec3(-91.5, -1, 0), vec3(-91.5, 15.25 - 1, 0), vec3(0,0,1), 0.5, vec4(1,1,1,1)));
    line->draw(_shader, model);
    for(int i = 1; i < 79; i++) {
      line.reset(new Line(vec3(-91.5 + i * interval, -1, 0), vec3(-91.5 + i * interval, 15.25 - 1, 0), vec3(0,0,1), 0.1, vec4(0,0,0,1)));
      line->draw(_shader, model);
    }
    line.reset(new Line(vec3(91.5, -1, 0), vec3(91.5, 15.25 - 1, 0), vec3(0,0,1), 0.5, vec4(1,1,1,1)));
    line->draw(_shader, model);

    // horizontal lines
    double horizontal_interval = (15.25) / 10;
    line.reset(new Line(vec3(-91.5, -1, 0), vec3(91.5, -1, 0), vec3(0,0,1), 0.5, vec4(1,1,1,1)));
    line->draw(_shader, model);
    for(int i = 1; i < 9; i++) {
      line.reset(new Line(vec3(-91.5, i * horizontal_interval - 1, 0), vec3(91.5, i * horizontal_interval -1, 0), vec3(0,0,1), 0.1, vec4(0,0,0,1)));
      line->draw(_shader, model);
    }
    line.reset(new Line(vec3(-91.5, 15.25 - 1, 0), vec3(91.5, 15.25 - 1, 0), vec3(0,0,1), 0.5, vec4(1,1,1,1)));
    line->draw(_shader, model);


    // Draw the ball
    ballShadow->draw(_shader,ballShadowFrame);
    if(ballFrame[3][1] < 2.0) {
        // draw the ball as if it's above the table if it's really inside of it
        float depth = 2.0 - ballFrame[3][1];
        mat4 drawBallFrame = glm::translate(ballFrame, vec3(0, depth, 0));
        ball->draw(_shader, drawBallFrame);

    } else {
        ball->draw(_shader, ballFrame);

    }
    // Draw the paddle using two cylinders
    paddle->draw(_shader, paddleFrame);
    handle->draw(_shader, paddleFrame);
    paddleShadow->draw(_shader,paddleShadowFrame);


}

void App::reloadShaders()
{
    _shader.compileShader("texture.vert", GLSLShader::VERTEX);
    _shader.compileShader("texture.frag", GLSLShader::FRAGMENT);
    _shader.link();
    _shader.use();
}
