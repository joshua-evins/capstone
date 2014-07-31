This project recursively stamps white circles into a black image buffer using OpenCL.

"Why Phi?"
Originally the circles were related by the golden ratio (phi) instead of by powers of two.

This is my lowest development time project at 5 days mostly during transit to / from work.

This project was also dropped the moment I got what I wanted out of it (a desktop background), so the settings are very specific.

To push OpenCL I forced the program to check every pixel in the entire image (instead of just a box around the circle) for being inside of the ring that makes the border of the circle. OpenCL did very well, as it tends to do for brute-force floating-point problems, and clocked in at around 2000x faster than the CPU implementation on my machine.

Demo Controls:
* Press ENTER in the black screen to start
* Look at the .png output file

Note: This project was developed in a rush, so I used my project template to generate it, which is why the OpenGL window is there even though the program displays nothing and outputs a file.