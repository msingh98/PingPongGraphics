/**
@file App.h

The default starter app is configured for OpenGL 3.3 and relatively recent
GPUs.
*/
#ifndef App_h
#define App_h

#include <api/MinVR.h>
using namespace MinVR;

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

// OpenGL Headers
#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#include <BasicGraphics.h>
using namespace basicgraphics;

class App : public VRApp {
public:

    App(int argc, char** argv);
    virtual ~App();

    /** USER INTERFACE CALLBACKS **/
    virtual void onAnalogChange(const VRAnalogEvent &state) override;
    virtual void onButtonDown(const VRButtonEvent &state) override;
    virtual void onButtonUp(const VRButtonEvent &state) override;
    virtual void onCursorMove(const VRCursorEvent &state) override;


    /** RENDERING CALLBACKS **/
    virtual void onRenderGraphicsScene(const VRGraphicsState& state) override;
    virtual void onRenderGraphicsContext(const VRGraphicsState& state) override;

protected:
    // Use these functions to access the current state of the paddle!
    glm::vec3 getPaddlePosition() { return glm::column(paddleFrame, 3); }
    glm::vec3 getPaddleNormal() { return glm::vec3(0,0,-1); }
    glm::vec3 getPaddleVelocity() { return paddleVel; }
    bool isOnTable();
    bool hitNet();
    bool hitPaddle();

    // The paddle is drawn with two cylinders
    std::unique_ptr<Cylinder> paddle;
    std::unique_ptr<Cylinder> handle;
    std::unique_ptr<Box> table;
    std::unique_ptr<Line> line;
    std::unique_ptr<Sphere> ball;
    std::unique_ptr<Line> tableLines;
    std::unique_ptr<Cylinder> ballShadow;
    std::unique_ptr<Cylinder> paddleShadow;




    // This 4x4 matrix stores position and rotation data for the paddle.
    glm::mat4 paddleFrame, lastPos , curPos;
    glm::mat4 ballFrame;
    glm::mat4 ballShadowFrame;
    glm::mat4 paddleShadowFrame;

    // This vector stores the paddle's current velocity.
    glm::vec3 paddleVel;
    glm::vec3 ballVel;

    double lastTime;
    double curFrameTime;

    virtual void reloadShaders();
    GLSLProgram _shader;

};


#endif
