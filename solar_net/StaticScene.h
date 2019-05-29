//
// Copyright (c) 2008-2015 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "Sample.h"

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <string>


namespace Urho3D
{

class Node;
class Scene;

}

struct _directions
{
	char *n; int nt;
	char *e; int et;
	char *s; int st;
	char *w; int wt;
} ;

/// Static 3D scene example.
/// This sample demonstrates:
///     - Creating a 3D scene with static content
///     - Displaying the scene using the Renderer subsystem
///     - Handling keyboard and mouse input to move a freelook camera
class StaticScene : public Sample
{
    URHO3D_OBJECT(StaticScene, Sample);

public:
    /// Construct.
    StaticScene(Context* context);

    /// Setup after engine initialization and before running the main loop.
    virtual void Start();

private:
    /// Construct the scene content.
    void CreateScene();
    /// Construct an instruction text to the UI.
    void CreateInstructions();
    /// Set up a viewport for displaying the scene.
    void SetupViewport();
    /// Read input and moves the camera.
    void MoveCamera(float timeStep);
    /// Subscribe to application-wide logic update events.
    void SubscribeToEvents();
    /// Handle the logic update event.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Manage joystick.
    void ManageJoystick(float timeStep);

        void HandleNetworkMessage(StringHash eventType, VariantMap& eventData);
        void HandleClientConnected(StringHash eventType, VariantMap& eventData);
        void HandleClientDisconnected(StringHash eventType, VariantMap& eventData);


    void CreateObject(char* uniqname, Vector3& pos, Vector3& scale, Quaternion& quat, char *model, char *material1,char *material2, int visible);
    void CreateObjectAtPoint(char *uniqname, char *pointname, Vector3& scale, Quaternion& quat, char *model, char *material1, char *material2, int visible);

    void CreateObjectFromString(char *str);
    Vector3 *CreatePoint(char* uniqname, Vector3 *pos);
    Vector3 *CreatePointFromString(char *str);
    void CreateObjectAtPointFromString(char *command);
    void moveObjectToPointFromString(char *command);
    void moveObjectToPoint(char *uniqname, char *pointname);

    ResourceCache *cache;
    std::map<std::string, Node*> nodeMap;
    std::map<std::string, Vector3*> pointMap;

    Input* input;
    int nbJoysticks;
    JoystickState* js;
    int xpos, ypos;
    Node *posNode;
    struct _directions possibleDirections[30];
    int cursorLocation;

    Node *earthPosNode;
    Node *sunPosNode;
    Node *sunPosRotNode;

    int myPort;
    int myAngle;
};
