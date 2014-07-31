This project is a fluid simulation using Smoothed Particle Hydrodynamics. The implementation is entirely GPU-based, using parallel processing and voxelization to bring the simulation into real time.

Demo Controls:
* Press ENTER in the black screen to start
* Hold the right mouse button and move the mouse to rotate the camera
* Standard WASD for camera movement
* R: move straight up
* F: move straight down
* Q: rotate spout counter-clockwise
* E: rotate spout clockwise
* IJKL: move the drain

This project is not 100% my own.

From AMD's SPH demo I used:
* Physics
  * Constants
  * Formulas
  * Implementation
* Voxelization process

My parts in this project were:
* The Spout and Drain, including key controls
  * OpenCL has no random utility, so I created a seeded random function that gave a good spread of random numbers based on a narrow spread of position values
  * This system will very rarely cause a crash if two particles are placed almost exactly on top of one another
    * I have considered moving from a position-seeded random to a global value that is atomically-incremented in order to prevent this from happening
    * I tested the approach of checking the positions of all other particles to ensure valid placement, but that solution slowed down the simulation significantly
    * I have also considered placing a sanity check in the malfunctioning kernel to reset any particles in an error state, but I want to prevent the error from occuring instead of just band-aid fixing it
* Rendering
  * AMD used a water-like rendering process that obscured the particle-based nature of the simulation
  * I opted to use my engine to render the simulation as simple particles to be more transparent, as well as to save some time to allow for more tinkering and optimization
* Sorting
  * I wrote a parallel Radix Sort, which increased the speed of the simulation
  
The .cl file that contains the kernels is heavily doctored with unraveled loops, vector-based loading, and a general lack of functions due partially to the nature of the OpenCL kernel language and partially to me testing different (and not always readable or extensible) approaches to see what gave the most performance.

One of my most interesting finds was in a situation where I needed to find the total number of ones before the nth element in an array of 1024 ones or zeroes. I knew the total number of ones in the array, so counting left from the nth element or right from the nth element didn't matter to me (total - right = left). However, the program was significantly slower when I included a conditional statement to determine which direction (left or right) would be faster and count that way. Branching is much more expensive in OpenCL than I expected.