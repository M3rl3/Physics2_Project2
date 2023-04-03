# Physics2_Project2

Project-2 for Physics & Simulation 2 (INFO-6022)

Note: this is an individual submission.

## Build Instructions:
- Built using Visual Studio 17 (2022) - Retarget solution if necessary.
- All build requirements and other files are included within the project.

## General Information:
- This project uses a third-party physics library (bullet).
- Solution has 2 projects within it. Where one of them is a static library (.lib) project and the other is the actual application (.exe).
- The static library project (PhysicsEngine) is added as a reference for the client application.
- The client application (Physics2_Project2) only includes the physics interfaces and the factory classes.
- The .exe project (Physics2_Project2) should be set as the startup project which depends on the .lib project (PhysicsEngine).

## Scene Information:
- The camera may be pointing in a direction slightly off from the scene, user can move the mouse around to reorient the camera.
- The scene has a floor and a bunch of cylinders arranged as bowling pins.
- User can instantiate a sphere on keypress which will then drop on the floor from a height.
- The user can control a single sphere in the scene at a time.
- That specific sphere instance will be colored bright yellow to indicate so.

## Controls:
- The player controllable sphere can be moved by using the W,A,S,D keys. These inputs apply a force on the ball based on the direction the camera is looking in.
- The camera can be controlled by the keyboard directional keys (UP, DOWN, LEFT, RIGHT) as well as (PAGEUP, PAGEDOWN) for y-axis.
- The camera can be panned around by moving the mouse.
- Pressing the SPACEBAR will create a sphere in a random location above the plane.
- Pressing the DELETE key will remove all the spheres that were instantiated.
- Pressing the R key will do a hard reset (Remove spheres + reset pre-existing objects).
- The EQUAL and MINUS keys can be used to change the sphere currently under the user's control.
- Pressing and holding LEFT-ALT will make the mouse cursor visible (minimize/maximize window).