<p>This project is a GPU-accelerated / parallel gravitational simulation using OpenCL. There are 8192 particles with O(N^2) inter-particle gravitation, for a total of 8192^2 - 8192 (67 million or so) calculations every frame.</p>

<p>Demo Controls:</p>
<ul>
<li>Press ENTER in the black screen to start</li>
<li>Hold the right mouse button and move the mouse to rotate the camera</li>
<li>Standard WASD for camera movement</li>
<li>R: move straight up</li>
<li>F: move straight down</p></li>
</ul>

<p>The engine has a slight conflict with the 2012 compiler, so upgrading that project, which is a requirement of this project, may cause a crash.</p>
<p>OpenCL is touchy across different hardware, so it is possible that may cause the program to break as well.</p>
