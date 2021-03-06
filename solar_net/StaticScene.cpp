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

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>

#include "StaticScene.h"
#include "Rotator.h"

#include <Urho3D/DebugNew.h>

const int MSG_GAME = 32;
const unsigned short GAME_SERVER_PORT = 32000;

URHO3D_DEFINE_APPLICATION_MAIN(StaticScene)

StaticScene::StaticScene(Context* context) :
    Sample(context)
{

	//myPort=0;
    //myAngle=0;
    context->RegisterFactory<Rotator>();
    const Vector<String>& arguments=GetArguments();

   sscanf(arguments[0].CString(),"%d",&myPort);
   sscanf(arguments[1].CString(),"%d",&myAngle);

   printf("myPort=%d myAngle=%d\n",myPort, myAngle);

}

void StaticScene::Start()
{
    // Execute base class startup
    Sample::Start();

    SetLogoVisible(false);

    cache = GetSubsystem<ResourceCache>();

    input = GetSubsystem<Input>();
    nbJoysticks=input->GetNumJoysticks();
    if (nbJoysticks>0)
	js=input->GetJoystickByIndex(0);
    else
	js=NULL;

    if (nbJoysticks>0)
	printf("Il y a un joystick.\n");

    pitch_ = 60.0f;

    xpos=1;
    ypos=1;

    cursorLocation=0;

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateInstructions();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to the frame update events
    SubscribeToEvents();
}

void StaticScene::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create the Octree component to the scene. This is required before adding any drawable components, or else nothing will
    // show up. The default octree volume will be from (-1000, -1000, -1000) to (1000, 1000, 1000) in world coordinates; it
    // is also legal to place objects outside the volume but their visibility can then not be checked in a hierarchically
    // optimizing manner
    scene_->CreateComponent<Octree>();

    Node* planeNode = scene_->CreateChild("Plane");
    planeNode->SetScale(Vector3(70.0f, 7.0f, 70.0f));
    /*StaticModel* planeObject = planeNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("bin/Data/Models/Disk.mdl"));
    planeObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/GreenTransparent.xml"));
*/
    /*Création Soleil*/
    sunPosNode = scene_->CreateChild("SunPos");
    sunPosNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Node* earth_revol = sunPosNode->CreateChild("SunPosRot");
    earth_revol->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Rotator* rotator_earth_revol = earth_revol->CreateComponent<Rotator>();
    rotator_earth_revol->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));
    Node* Sun=sunPosNode->CreateChild("Sun");
    Sun->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    Sun->SetScale(Vector3(40.0f, 40.0f, 40.0f));
    StaticModel* sunObject = Sun->CreateComponent<StaticModel>();
    sunObject->SetModel(cache->GetResource<Model>("bin/Data/Models/Sphere.mdl"));
    sunObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/sunmap.xml"));

    
    /*Création Terre*/
    earthPosNode = earth_revol->CreateChild("EarthPos");
    earthPosNode->SetPosition(Vector3(50.0f, 0.0f, 0.0f));//distance Terre_Soleil
    //earthPosNode->SetScale(Vector3(1.0f, 1.0f, 1.0f));
    //Rotator* rotatorEarthPos = earthPosNode->CreateComponent<Rotator>();
    //rotatorEarthPos->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));
    Node* earthInclinedNode = earthPosNode->CreateChild("EarthInclined");
    earthInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    earthInclinedNode->SetRotation(Quaternion(0.0f, 0.0f, 23.0f)); //Inclinaison de 23° par rapport à l'écliptique

    Node* cylinderInclinedNode = earthInclinedNode->CreateChild("cylinderInclined");//Axe de rotation
    cylinderInclinedNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    cylinderInclinedNode->SetScale(Vector3(0.50f, 12.0f, 0.50f));
    StaticModel* cylinderInclinedObject = cylinderInclinedNode->CreateComponent<StaticModel>();
    cylinderInclinedObject->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));
    cylinderInclinedObject->SetMaterial(cache->GetResource<Material>("Materials/cyl10.xml"));
    Node* earthNode = earthInclinedNode->CreateChild("Earth");
    //Node* earthNode = earthPosNode->CreateChild("Earth");
    earthNode->SetScale(Vector3(10.0f, 10.0f, 10.0f));
    StaticModel* earthObject = earthNode->CreateComponent<StaticModel>();
    earthObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    earthObject->SetMaterial(cache->GetResource<Material>("Materials/earthmap.xml"));
    Rotator* earthnoderotator = earthNode->CreateComponent<Rotator>();
    earthnoderotator->SetRotationSpeed(Vector3(0.0f, -100.0f, 0.0f));

    Node* moon_revol = earthPosNode->CreateChild("EarthPosRot");
    moon_revol->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    moon_revol->SetRotation(Quaternion(0.0f, 0.0f, 5.16f));
    Rotator* rotator_moon_revol = moon_revol->CreateComponent<Rotator>();
    rotator_moon_revol->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));

    Node* moonNode = moon_revol->CreateChild("Moon");
    moonNode->SetPosition(Vector3(10.0f, 0.0f, 0.0f));
    moonNode->SetScale(Vector3(5.0f, 5.0f, 5.0f));
    moonNode->SetRotation(Quaternion(0.0f, 0.0f, 6.68f));
    Rotator* moon_rot= moonNode->CreateComponent<Rotator>();
    moon_rot->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));
    /*
    Node* moonrotNode=moonNode->CreateChild("Moonrot");
    Rotator* rotatormoon = moonrotNode->CreateComponent<Rotator>();
    rotatormoon->SetRotationSpeed(Vector3(0.0f, -30.0f, 0.0f));*/

    StaticModel* moonObject = moonNode->CreateComponent<StaticModel>();
    moonObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    moonObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/moonmap.xml"));

    
    /*MERCURE*/
    Node* mercure_revol= sunPosNode->CreateChild("revol_mercure"); //Révolution autour du soleil
    Rotator* rotator_mercure_revol= mercure_revol->CreateComponent<Rotator>();
    rotator_mercure_revol->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));

    Node* MercureNode = mercure_revol->CreateChild("MercurePosNode");
    MercureNode->SetPosition(Vector3(10.0f, 0.0f, 0.0f)); //Distance Soleil
    MercureNode->SetScale(Vector3(5.0f, 5.0f, 5.0f)); //Taille
    Rotator* mercure_rot= MercureNode->CreateComponent<Rotator>();
    mercure_rot->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f)); //Vitesse rotation
    StaticModel* mercureObject = MercureNode->CreateComponent<StaticModel>();
    mercureObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    mercureObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/mercurymap.xml"));


    /*VENUS*/
    Node* venus_revol= sunPosNode->CreateChild("revol_venus"); 
    venus_revol->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    venus_revol->SetRotation(Quaternion(0.0f, 0.0f, 3.39f));//inclinaison par rapport à l'axe de l'écliptique
    Rotator* rotator_venus_revol= venus_revol->CreateComponent<Rotator>();
    rotator_venus_revol->SetRotationSpeed(Vector3(0.0f, 10.0f, 0.0f));//Revolution autour du Soleil

    Node* VenusNode=venus_revol->CreateChild("Venus_node");
    VenusNode->SetPosition(Vector3(40.0f, 0.0f, 0.0f));
    VenusNode->SetRotation(Quaternion(0.0f, 0.0f, 177.36f));
    VenusNode->SetScale(Vector3(5.0f, 5.0f, 5.0f)); //Taille
    Rotator* venus_rot= VenusNode->CreateComponent<Rotator>();
    venus_rot->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f)); //Vitesse rotation
    StaticModel* venusObject = VenusNode->CreateComponent<StaticModel>();
    venusObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    venusObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/venusmap.xml"));

    /*MARS*/
    Node* mars_revol= sunPosNode->CreateChild("revol_mars"); 
    mars_revol->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    mars_revol->SetRotation(Quaternion(0.0f, 0.0f, 1.85f));//inclinaison par rapport à l'axe de l'écliptique
    Rotator* rotator_mars_revol= mars_revol->CreateComponent<Rotator>();
    rotator_mars_revol->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));//Revolution autour du Soleil

    Node* MarsNode=mars_revol->CreateChild("Mars_node");
    MarsNode->SetPosition(Vector3(70.0f, 0.0f, 0.0f));
    MarsNode->SetRotation(Quaternion(0.0f, 0.0f, 25.19f));
    MarsNode->SetScale(Vector3(5.0f, 5.0f, 5.0f)); //Taille
    Rotator* Mars_rot= MarsNode->CreateComponent<Rotator>();
    Mars_rot->SetRotationSpeed(Vector3(0.0f, -100.0f, 0.0f)); //Vitesse rotation
    StaticModel* MarsObject = MarsNode->CreateComponent<StaticModel>();
    MarsObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    MarsObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/marsmap.xml"));


    /*JUPITER*/
    Node* jupiter_revol= sunPosNode->CreateChild("revol_jupiter"); 
    jupiter_revol->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    jupiter_revol->SetRotation(Quaternion(0.0f, 0.0f, 1.3f));//inclinaison par rapport à l'axe de l'écliptique
    Rotator* rotator_jupiter_revol= jupiter_revol->CreateComponent<Rotator>();
    rotator_jupiter_revol->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));//Revolution autour du Soleil

    Node* JupiterNode=jupiter_revol->CreateChild("Jupiter_node");
    JupiterNode->SetPosition(Vector3(100.0f, 0.0f, 0.0f));
    JupiterNode->SetRotation(Quaternion(0.0f, 0.0f, 3.12f));
    JupiterNode->SetScale(Vector3(20.0f, 20.0f, 20.0f)); //Taille
    Rotator* Jupiter_rot= JupiterNode->CreateComponent<Rotator>();
    Jupiter_rot->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f)); //Vitesse rotation
    StaticModel* JupiterObject = JupiterNode->CreateComponent<StaticModel>();
    JupiterObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    JupiterObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/jupitermap.xml"));

    /*SATURNE*/
    Node* saturne_revol= sunPosNode->CreateChild("revol_saturne");  
    saturne_revol->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    saturne_revol->SetRotation(Quaternion(0.0f, 0.0f, 2.48f));//inclinaison par rapport à l'axe de l'écliptique
    Rotator* rotator_saturne_revol= saturne_revol->CreateComponent<Rotator>();
    rotator_saturne_revol->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));//Revolution autour du Soleil

    Node* SaturneNode=saturne_revol->CreateChild("Saturne_node");
    SaturneNode->SetPosition(Vector3(150.0f, 0.0f, 0.0f));
    SaturneNode->SetRotation(Quaternion(0.0f, 0.0f, 26.73f));
    SaturneNode->SetScale(Vector3(20.0f, 20.0f, 20.0f)); //Taille
    Rotator* Saturne_rot= SaturneNode->CreateComponent<Rotator>();
    Saturne_rot->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f)); //Vitesse rotation
    StaticModel* SaturneObject = SaturneNode->CreateComponent<StaticModel>();
    SaturneObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    StaticModel* SaturneAnneau = SaturneNode->CreateComponent<StaticModel>();
    SaturneObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/saturnmap.xml"));
    SaturneAnneau->SetModel(cache->GetResource<Model>("Models/Disk.mdl"));
    SaturneAnneau->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/saturnringmap.xml"));


    /*URANUS*/
    Node* uranus_revol= sunPosNode->CreateChild("revol_uranus");  
    uranus_revol->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    uranus_revol->SetRotation(Quaternion(0.0f, 0.0f, 0.77f));//inclinaison par rapport à l'axe de l'écliptique
    Rotator* rotator_uranus_revol= uranus_revol->CreateComponent<Rotator>();
    rotator_uranus_revol->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));//Revolution autour du Soleil

    Node* UranusNode=uranus_revol->CreateChild("Uranus_node");
    UranusNode->SetPosition(Vector3(200.0f, 0.0f, 0.0f));  
    UranusNode->SetRotation(Quaternion(0.0f, 0.0f, 97.77f));
    UranusNode->SetScale(Vector3(20.0f, 20.0f, 20.0f)); //Taille
    Rotator* Uranus_rot= UranusNode->CreateComponent<Rotator>();
    Uranus_rot->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f)); //Vitesse rotation
    StaticModel* UranusObject = UranusNode->CreateComponent<StaticModel>();
    UranusObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    UranusObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/uranusmap.xml"));


    /*NEPTUNE*/
    Node* neptune_revol= sunPosNode->CreateChild("revol_neptune");  
    neptune_revol->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    neptune_revol->SetRotation(Quaternion(0.0f, 0.0f, 1.76f));//inclinaison par rapport à l'axe de l'écliptique
    Rotator* rotator_neptune_revol= neptune_revol->CreateComponent<Rotator>();
    rotator_neptune_revol->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));//Revolution autour du Soleil

    Node* NeptuneNode=neptune_revol->CreateChild("neptune_node");
    NeptuneNode->SetPosition(Vector3(250.0f, 0.0f, 0.0f));
    NeptuneNode->SetRotation(Quaternion(0.0f, 0.0f, 28.3f));
    NeptuneNode->SetScale(Vector3(20.0f, 20.0f, 20.0f)); //Taille
    Rotator* Neptune_rot= NeptuneNode->CreateComponent<Rotator>();
    Neptune_rot->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f)); //Vitesse rotation
    StaticModel* NeptuneObject = NeptuneNode->CreateComponent<StaticModel>();
    NeptuneObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    NeptuneObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/neptunemap.xml"));


    /*PLUTON*/
    Node* pluton_revol= sunPosNode->CreateChild("revol_pluton");  
    pluton_revol->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    pluton_revol->SetRotation(Quaternion(0.0f, 0.0f, 0.77f));//inclinaison par rapport à l'axe de l'écliptique
    Rotator* rotator_pluton_revol= pluton_revol->CreateComponent<Rotator>();
    rotator_pluton_revol->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f));//Revolution autour du Soleil

    Node* PlutonNode=pluton_revol->CreateChild("pluton_node");
    PlutonNode->SetPosition(Vector3(300.0f, 0.0f, 0.0f));
    PlutonNode->SetRotation(Quaternion(0.0f, 0.0f, 97.77f));
    PlutonNode->SetScale(Vector3(20.0f, 20.0f, 20.0f)); //Taille
    Rotator* Pluton_rot= PlutonNode->CreateComponent<Rotator>();
    Pluton_rot->SetRotationSpeed(Vector3(0.0f, -10.0f, 0.0f)); //Vitesse rotation
    StaticModel* PlutonObject = PlutonNode->CreateComponent<StaticModel>();
    PlutonObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    PlutonObject->SetMaterial(cache->GetResource<Material>("bin/Data/Materials/plutonmap.xml"));



    /*StaticModel* SaturneAnneau = SaturneNode->CreateComponent<StaticModel>();
    SaturneAnneau->SetModel(cache->GetResource<Model>("Models/Disk.mdl"));
*/

    //moonObject->SetMaterial(cache->GetResource<Material>("Materials/earthmap.xml"));
    //Rotator* rotatorMoon = moonNode->CreateComponent<Rotator>();
    //rotatorMoon->SetRotationSpeed(Vector3(0.0f, -50.0f, 0.0f));

    // Create a directional light to the world so that we can see something. The light scene node's orientation controls the
    // light direction; we will use the SetDirection() function which calculates the orientation from a forward direction vector.
    // The light will use default settings (white light, no shadows)
    


    Node* lightNodecentr = scene_->CreateChild();
    lightNodecentr->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    //lightNode->SetDirection(Vector3(0.0f, 0.0f, 0.0f)); // The direction vector does not need to be normalized
    Light* lightcentre = lightNodecentr->CreateComponent<Light>();
    //light->SetLightType(LIGHT_DIRECTIONAL);
    lightcentre->SetBrightness(7.0);

    Node* lightNodeleft = scene_->CreateChild();
    lightNodeleft->SetPosition(Vector3(7.0f, 0.0f, 0.0f));
    //lightNode->SetDirection(Vector3(0.0f, 0.0f, 0.0f)); // The direction vector does not need to be normalized
    Light* lightleft = lightNodeleft->CreateComponent<Light>();
    //light->SetLightType(LIGHT_DIRECTIONAL);
    lightleft->SetBrightness(7.0);


    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f)); // The direction vector does not need to be normalized
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    //light->SetBrightness(3.0);

    // Create a scene node for the camera, which we will move around
    // The camera will use default settings (1000 far clip distance, 45 degrees FOV, set aspect ratio automatically)
    cameraNode_ = scene_->CreateChild("Camera");
    cameraNode_->CreateComponent<Camera>();

    // Set an initial position for the camera scene node above the plane
    cameraNode_->SetPosition(Vector3(0.0f, 15.0f, 0.0f));
}

void StaticScene::CreateInstructions()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

/*
    // Construct new Text object, set string to display and font to use
    Text* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText("Use WASD keys and mouse/touch to move");
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
*/
}

void StaticScene::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen. We need to define the scene and the camera
    // at minimum. Additionally we could configure the viewport screen size and the rendering path (eg. forward / deferred) to
    // use, but now we just use full screen and default render path configured in the engine command line options
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}

void StaticScene::MoveCamera(float timeStep)
{
    // Do not move if the UI has a focused element (the console)
    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    Input* input = GetSubsystem<Input>();

    // Movement speed as world units per second
    const float MOVE_SPEED = 20.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    //pitch_ = 60.0f;

    pitch_ = Clamp(pitch_, -90.0f, 90.0f);
    
    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    // Use the Translate() function (default local space) to move relative to the node's orientation.
    if (input->GetKeyDown('W'))
        cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('S'))
        cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('A'))
        cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('D'))
        cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

/*
    //Vector3 cpos=cameraNode_->GetPosition();
    Vector3 cpos=earthPosNode->GetPosition();
    if (input->GetKeyDown('J'))
	{
                float dx=MOVE_SPEED * timeStep;
                cpos.x_=cpos.x_+dx;
                //cameraNode_->SetPosition(cpos);
                earthPosNode->SetPosition(cpos);
        	//cameraNode_->Translate(Vector3(MOVE_SPEED * timeStep,0.0f,0.0f));
	}
    if (input->GetKeyDown('H'))
	{
                float dx=MOVE_SPEED * timeStep;
                cpos.x_=cpos.x_-dx;
                //cameraNode_->SetPosition(cpos);
                earthPosNode->SetPosition(cpos);
        	//cameraNode_->Translate(Vector3(MOVE_SPEED * timeStep,0.0f,0.0f));
	}
        //cameraNode_->Translate(Vector3(-MOVE_SPEED * timeStep,0.0f,0.0f));
    if (input->GetKeyDown('U'))
        {
                float dz=MOVE_SPEED * timeStep;
                cpos.z_=cpos.z_+dz;
                //cameraNode_->SetPosition(cpos);
                earthPosNode->SetPosition(cpos);
        }
        //cameraNode_->Translate(Vector3(0.0f,MOVE_SPEED * timeStep,0.0f));
    if (input->GetKeyDown('N'))
        {
                float dz=-MOVE_SPEED * timeStep;
                cpos.z_=cpos.z_+dz;
                //cameraNode_->SetPosition(cpos);
                earthPosNode->SetPosition(cpos);
        }

	if (nbJoysticks>0)
		ManageJoystick(timeStep);
*/


     if (input->GetKeyPress('I'))
        {
        printf("I\n");
        myAngle-=36;
        printf("myAngle=%d\n",myAngle);
        cameraNode_->SetRotation(Quaternion(pitch_, (float)myAngle, 0.0f));
    }else if (input->GetKeyPress('O'))
    {
        printf("O\n");
        myAngle+=36;
        printf("myAngle=%d\n",myAngle);
        cameraNode_->SetRotation(Quaternion(pitch_, (float)myAngle, 0.0f));
    }else{
         // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    
        cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
    }
}


   

void StaticScene::ManageJoystick(float timeStep)
{
    const float MOVE_SPEED = 20.0f;
    Vector3 cpos=cameraNode_->GetPosition();
    //Vector3 posn=posNode->GetPosition();

	double xJ0=js->GetAxisPosition(0);
	double yJ0=js->GetAxisPosition(1);
	double xJ1=js->GetAxisPosition(2);
	double yJ1=js->GetAxisPosition(3);

	    if (xJ0>0.1)
        	{
                	float dx=MOVE_SPEED* xJ0 * timeStep;
                	cpos.x_=cpos.x_+dx;
                	cameraNode_->SetPosition(cpos);
        	}
	    if (xJ0<-0.1)
        	{
                	float dx=MOVE_SPEED * xJ0 * timeStep;
                	cpos.x_=cpos.x_+dx;
                	cameraNode_->SetPosition(cpos);
        	}
	    if (yJ0<-0.1)
        	{
                	float dz=MOVE_SPEED * yJ0 * timeStep;
                	cpos.z_=cpos.z_-dz;
                	cameraNode_->SetPosition(cpos);
        	}
	    if (yJ0>0.1)
        	{
                	float dz=MOVE_SPEED * yJ0 * timeStep;
                	cpos.z_=cpos.z_-dz;
                	cameraNode_->SetPosition(cpos);
        	}

/*
	printf("Buttons=%d\n",js->GetNumButtons());
	printf("01234567890123456789\n");
	printf("00000000001111111111\n");
	for (int i=0;i<js->GetNumButtons();i++)
		if (js->GetButtonDown(i)==true)
			printf("1");
		else
			printf("0");
	printf("\n");
	printf("Hats=%d\n",js->GetNumHats());

	printf("%f %f\n", (float)js->GetAxisPosition(2), (float)js->GetAxisPosition(3)); 
*/
	
	    if (xJ1>0.1)
        	{
                	float dy=MOVE_SPEED * 2.0 * xJ1 * timeStep;
                	cpos.y_=cpos.y_-dy;
                	cameraNode_->SetPosition(cpos);
        	}
	    if (xJ1<-0.1)
        	{
                	float dy=MOVE_SPEED * 2.0 * xJ1 * timeStep;
                	cpos.y_=cpos.y_-dy;
                	cameraNode_->SetPosition(cpos);
        	}

	    if (yJ1>0.1)
        	{
                	float amount=MOVE_SPEED * 2.0 * yJ1 * timeStep;
			printf("amount=%f\n",amount);
			pitch_ +=amount;
        	}
	    if (yJ1<-0.1)
        	{
                	float amount=MOVE_SPEED * 2.0 * yJ1 * timeStep;
			printf("amount=%f\n",amount);
			pitch_ +=amount;
        	}

	if (js->GetButtonPress(13))
        	{
			std::cout << "--------------------------------------------------------- bouton 13" << std::endl;
			if (possibleDirections[cursorLocation].wt!=-1)
			{
				moveObjectToPoint((char*)"MoralePawn",possibleDirections[cursorLocation].w);
				cursorLocation=possibleDirections[cursorLocation].wt;
			}
        	}
	if (js->GetButtonPress(14))
        	{
			std::cout << "--------------------------------------------------------- bouton 14" << std::endl;
			if (possibleDirections[cursorLocation].et!=-1)
			{
				moveObjectToPoint((char*)"MoralePawn",possibleDirections[cursorLocation].e);
				cursorLocation=possibleDirections[cursorLocation].et;
			}
        	}
	if (js->GetButtonPress(11))
        	{
			std::cout << "--------------------------------------------------------- bouton 11" << std::endl;
			if (possibleDirections[cursorLocation].nt!=-1)
			{
				moveObjectToPoint((char*)"MoralePawn",possibleDirections[cursorLocation].n);
				cursorLocation=possibleDirections[cursorLocation].nt;
			}
        	}
	if (js->GetButtonPress(12))
        	{
			std::cout << "--------------------------------------------------------- bouton 12" << std::endl;
			if (possibleDirections[cursorLocation].st!=-1)
			{
				moveObjectToPoint((char*)"MoralePawn",possibleDirections[cursorLocation].s);
				cursorLocation=possibleDirections[cursorLocation].st;
			}
        	}

    	pitch_ = Clamp(pitch_, -90.0f, 90.0f);

	if (js->GetButtonPress(4))
        	{
                	cpos.x_=0.0;
                	cpos.y_=15.0;
                	cpos.z_=0.0;
			pitch_=60.0;
                	cameraNode_->SetPosition(cpos);
        	}

}

void StaticScene::SubscribeToEvents()
{
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(StaticScene, HandleUpdate));

        // Start server

        Network* network = GetSubsystem<Network>();
        network->StartServer(GAME_SERVER_PORT);

        // Subscribe to network events

        SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(StaticScene, HandleClientConnected));
        SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(StaticScene, HandleClientDisconnected));
        SubscribeToEvent(E_NETWORKMESSAGE, URHO3D_HANDLER(StaticScene, HandleNetworkMessage));

}

void StaticScene::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move the camera, scale movement with time step
    MoveCamera(timeStep);
}

void StaticScene::HandleClientConnected(StringHash eventType, VariantMap& eventData)
{
        printf("Client connected\n");
}

void StaticScene::HandleClientDisconnected(StringHash eventType, VariantMap& eventData)
{
        printf("Client disconnected\n");
}

void StaticScene::HandleNetworkMessage(StringHash eventType, VariantMap& eventData)
{
        float vx,vz;

        Network* network = GetSubsystem<Network>();

        using namespace NetworkMessage;

        int msgID = eventData[P_MESSAGEID].GetInt();
        Connection* remoteSender = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());

       
        std::cout << "HandleNetworkMessage" << std::endl;

        if (msgID == MSG_GAME) 
        {
                const PODVector<unsigned char>& data = eventData[P_DATA].GetBuffer();
                // Use a MemoryBuffer to read the message data so that there is no unnecessary copying
                MemoryBuffer msg(data);
                String text = msg.ReadString();
                char s[100],com[100];
                int tx,tz,tnum,torient;
                strcpy(s,text.CString());
                printf("Message received:%s\n",s);
        
        // RAJOUTER DU CODE POUR DECODER ET GERER LES MESSAGES DU CLIENT

            if (s[0]=='X')
            {
            }
        }
}


// ===================================================================

void StaticScene::CreateObject(char *uniqname,
        Vector3& pos, Vector3& scale, Quaternion& quat,
        char *model, char *material1, char *material2, int visible)
{

        Node* oNode = scene_->CreateChild(uniqname);
        oNode->SetPosition(pos);
        oNode->SetScale(scale);
        oNode->SetRotation(quat);
        StaticModel* oObject = oNode->CreateComponent<StaticModel>();
        char oModel[100];
        sprintf(oModel,"Models/%s",model);
        oObject->SetModel(cache->GetResource<Model>(oModel));
        char oMaterial1[100];
        sprintf(oMaterial1,"Materials/%s",material1);
        Material *mm1=cache->GetResource<Material>(oMaterial1);
        oObject->SetMaterial(cache->GetResource<Material>(oMaterial1));
        char oMaterial2[100];
        sprintf(oMaterial2,"Materials/%s",material2);
        Material *mm2=cache->GetResource<Material>(oMaterial2);

        oObject->SetMaterial(mm1);

        if (visible==1)
        {
                oObject->SetMaterial(mm1);
                //oNode->SetEnabled(true);
        }
        else
        {
                oObject->SetMaterial(mm2);
                //oNode->SetEnabled(false);
        }

        nodeMap.insert(std::make_pair(uniqname,oNode));
}

void StaticScene::CreateObjectAtPoint(char *uniqname, char *pointname,
        Vector3& scale, Quaternion& quat,
        char *model, char *material1, char *material2, int visible)
{
        Node* oNode = scene_->CreateChild(uniqname);
	Vector3 *n=pointMap[pointname];
        oNode->SetPosition(*n);
        oNode->SetScale(scale);
        oNode->SetRotation(quat);
        StaticModel* oObject = oNode->CreateComponent<StaticModel>();
        char oModel[100];
        sprintf(oModel,"Models/%s",model);
        oObject->SetModel(cache->GetResource<Model>(oModel));
        char oMaterial1[100];
        sprintf(oMaterial1,"Materials/%s",material1);
        Material *mm1=cache->GetResource<Material>(oMaterial1);
        oObject->SetMaterial(cache->GetResource<Material>(oMaterial1));
        char oMaterial2[100];
        sprintf(oMaterial2,"Materials/%s",material2);
        Material *mm2=cache->GetResource<Material>(oMaterial2);

        oObject->SetMaterial(mm1);

        if (visible==1)
        {
                oObject->SetMaterial(mm1);
        }
        else
        {
                oObject->SetMaterial(mm2);
        }

        nodeMap.insert(std::make_pair(uniqname,oNode));
}

void StaticScene::CreateObjectFromString(char *command)
{
        float posx, posy, posz;
        float scalex, scaley, scalez;
        float quatx, quaty, quatz;
        char uniqname[100];
        char model[100];
        char material1[100];
        char material2[100];
        int vis;

        sscanf(command+3,"%s %f %f %f %f %f %f %f %f %f %s %s %s %d",
                uniqname, &posx, &posy, &posz, &scalex, &scaley, &scalez,
                &quatx, &quaty, &quatz, model, material1, material2, &vis);

        printf("CreateObjectFromString %s %f %f %f %f %f %f %f %f %f %s %s %s %d\n",
                uniqname, posx, posy, posz, scalex, scaley, scalez,
                quatx, quaty, quatz, model, material1, material2, vis);

        Vector3 pos(posx,posy,posz);
        Vector3 scale(scalex,scaley,scalez);
        Quaternion quat(quatx,quaty,quatz);

        CreateObject(uniqname,pos,scale,quat,model,material1,material2,vis);
}

void StaticScene::CreateObjectAtPointFromString(char *command)
{
        float scalex, scaley, scalez;
        float quatx, quaty, quatz;
        char uniqname[100];
        char pointname[100];
        char model[100];
        char material1[100];
        char material2[100];
        int vis;

        sscanf(command+3,"%s %s %f %f %f %f %f %f %s %s %s %d",
                uniqname, pointname, &scalex, &scaley, &scalez,
                &quatx, &quaty, &quatz, model, material1, material2, &vis);

        printf("CreateObjectAtPointFromString %s %s %f %f %f %f %f %f %s %s %s %d\n",
                uniqname, pointname, scalex, scaley, scalez,
                quatx, quaty, quatz, model, material1, material2, vis);

        Vector3 scale(scalex,scaley,scalez);
        Quaternion quat(quatx,quaty,quatz);

        CreateObjectAtPoint(uniqname,pointname,scale,quat,model,material1,material2,vis);
}

Vector3* StaticScene::CreatePoint(char *uniqname, Vector3 *pos)
{
        pointMap.insert(std::make_pair(uniqname,pos));
	return pos;
}

Vector3* StaticScene::CreatePointFromString(char *command)
{
        float posx, posy, posz;
        float scalex, scaley, scalez;
        float quatx, quaty, quatz;
        char uniqname[100];
        char model[100];
        char material1[100];
        char material2[100];
        int vis;

        sscanf(command+3,"%s %f %f %f",
                uniqname, &posx, &posy, &posz);

        printf("CreatePointFromString %s %f %f %f\n",
                uniqname, posx, posy, posz);

        Vector3 *pos=new Vector3(posx,posy,posz);

        return CreatePoint(uniqname,pos);
}

void StaticScene::moveObjectToPointFromString(char *command)
{
        char uniqname[100];
        char pointname[100];

        sscanf(command+3,"%s %s",
                uniqname, pointname);

        printf("moveObjectToPointFromString %s %s\n",
                uniqname, pointname);

	moveObjectToPoint(uniqname, pointname);
}

void StaticScene::moveObjectToPoint(char *uniqname, char *pointname)
{
	std::cout << "moveObjectToPoint " << uniqname << "," << pointname << std::endl;

        Node* oNode = nodeMap[uniqname];
	Vector3 *n=pointMap[pointname]; 
        oNode->SetPosition(*n);
}
