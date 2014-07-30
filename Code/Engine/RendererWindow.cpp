#include "RendererWindow.h"
#include "Renderer.h"
#include <iostream>

namespace Engine
{
	RendererWindow::RendererWindow()
	{
		elapsed = 0.0f;
		numFrames = 1;
		initialized = false;
		setCentralWidget(renderer = new Renderer());
		show();
		
		centralWidget()->setMinimumWidth(800);
		centralWidget()->setMinimumHeight(600);
		centralWidget()->show();

		startTimer(0); //target 60 fps
	}

	void RendererWindow::timerEvent(QTimerEvent*)
	{
		if(!initialized && GetAsyncKeyState(0x0D)) // ENTER
		{
			initialized = true;
			initialize();
			renderer->initializeSize();
		}
		if(initialized)
		{
			renderer->update();
			
			inheritedUpdate();
			renderer->repaint();
			
			if(true) // set to true for FPS tracking
			{
				if(numFrames > 100)
				{
					elapsed = elapsed / numFrames;
					numFrames = 2;
				}

				elapsed += clock.deltaTime();
				if(numFrames%10 == 0)
					std::cout << "average fps: " << 1.0f / (elapsed / numFrames) << std::endl;
				//std::cout << "pos: " << renderer->camera.position.x << ", " << renderer->camera.position.y << ", " << renderer->camera.position.z << std::endl;
				//std::cout << "facing: " << renderer->camera.facing.x << ", " << renderer->camera.facing.y << ", " << renderer->camera.facing.z << std::endl;
				numFrames++;
			}
		}
		clock.frameTick();
	}

	void RendererWindow::initialize()
	{
		
	}

	void RendererWindow::inheritedUpdate()
	{

	}
}