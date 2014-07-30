This project is a GPU-accelerated / parallel gravitational simulation using OpenCL. There are 8192 particles with O(N^2) inter-particle gravitation, for a total of 8192^2 - 8192 (67 million or so) calculations every frame.

Demo Controls:
* Press ENTER in the black screen to start
* Hold the right mouse button and move the mouse to rotate the camera
* Standard WASD for camera movement
* R: move straight up
* F: move straight down

The engine has a slight conflict with the 2012 compiler, so upgrading that project, which is a requirement of this project, may cause a crash.
OpenCL is touchy across different hardware, so it is possible that may cause the program to break as well.
