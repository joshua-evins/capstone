This repository contains the results of my capstone course and one passion project inspired by the course.

Capstone is a 10-week, self-managed course in which I selected my own project(s) and worked independently with one-week check-offs. All of these projects are solely for my personal development as a programmer and not intended for publishing or distribution.

Directories:

The Code directory contains:
* The build directory for all projects (LocalDeploy)
* The projects themselves
  * For more information about a project see its README file
	
The Documentation directory contains:
* Weekly Project Summary: a fairly detailed record of what I did, what issues I encountered, and some of my solutions and thought processes
* Timesheet: shows how much time I spent during a given week by day

The Vendor directory contains:
* Third-party resources
  * I reference the headers and lib files from here in my projects, but use copies of the .dll files next to the executables for simplicity.
  
Demo Controls:
* Press ENTER in the black screen to start
* Hold the right mouse button and move the mouse to rotate the camera
* Standard WASD for camera movement
* R: move straight up
* F: move straight down
  
All of the projects are tied to the engine, and the engine is tied to the Visual Studio 2010 compiler (there is a slight conflict with the 2012 compiler). 

OpenCL is touchy across different hardware, so NBody and SmoothedParticleHydrodynamics are not guaranteed to work on your machine (but there is a fair chance if you want to try).