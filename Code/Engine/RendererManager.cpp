#include "RendererManager.h"
#include "Qt\qdebug.h"
#include "DebugMenu.h"
#include <QtGui\qwidget.h>
#include <QtGui\QVBoxLayout>
#include <glm\gtx\transform.hpp>
#include <QtOpenGL\qglwidget>

namespace Engine
{
	void RendererManager::initialize(Renderer* rendererIn)
	{
		mainWindow = new QWidget;
		QVBoxLayout* mainLayout;
		mainWindow->setLayout(mainLayout = new QVBoxLayout);
		renderer = rendererIn;
#ifdef ENGINE_DEBUG
		debugInitialize(mainLayout);
#endif
		
		mainLayout->addWidget(renderer);

		mainWindow->show();
		renderer->setMinimumHeight(800);
		renderer->setMinimumWidth(1200);
		
		renderer->show();

		startTimer(10);
	}

	void RendererManager::childUpdate()
	{
		
	}

	void RendererManager::timerEvent( QTimerEvent* )
	{
		deltaTime = clock.deltaTime();
		clock.frameTick();
		renderer->update();
		childUpdate();

#ifdef ENGINE_DEBUG
		if(!renderer->debug_HalfScreen)
			renderer->camera.update();
		debugUpdate(deltaTime);
#endif
#ifndef ENGINE_DEBUG
		renderer->repaint();
#endif
	}

	

	#pragma region Debug
	#ifdef ENGINE_DEBUG

		void RendererManager::debugInitialize(QVBoxLayout* mainLayout)
		{
			QVBoxLayout* debugLayout;
			mainLayout->addLayout(debugLayout = new QVBoxLayout);
			debugShapeHandler.initialize(renderer);
			DebugMenu::getInstance().initialize(debugLayout, renderer);
			DebugMenu::getInstance().watchFloat("camera.x", reinterpret_cast<float*>(renderer->camera.debug_getCameraPosition()));
			DebugMenu::getInstance().slideFloat("cube1 position.y", reinterpret_cast<float*>(renderer->debug_getRenderables()[0].uniforms[0].data)+13, -30.0f, 30.0f);
			DebugMenu::getInstance().controlBool("Toggle Debug Camera", &(renderer->debug_HalfScreen));
			DebugMenu::getInstance().controlBool("Toggle paused (black screen)", &(renderer->paused));
		}

		void RendererManager::debugUpdate(float deltaTime)
		{
			if(renderer->debug_HalfScreen)
				renderer->debugCamera->update();
			static Renderer::UniformInfo* arrowTransform = NULL;
			static bool firstArr = true;
			debugShapeHandler.tickAllLifetimes(deltaTime);

			if ( GetAsyncKeyState(0x4A) & 0x8000 )
			{
				Renderer::Renderable* arr = debugShapeHandler.addArrow(0, 5.0f, true);
				if(firstArr)
					arrowTransform = arr->addUniform(Engine::Renderer::UniformFormat::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", 64, &xformMat[0][0]);
				else
				{
					glm::mat4 tempTransform(glm::translate(0.0f,3.0f,10.0f) * glm::rotate(40.0f, 1.0f, 0.0f, 0.0f));
					arr->addUniform(Engine::Renderer::UniformFormat::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", 64, &tempTransform);
				}
				firstArr = false;
				renderer->repaint();
			}

			if ( (GetAsyncKeyState(0x4B) & 0x8000 ) && debugShapeHandler.enoughTimeElapsedSinceLastAdd() )
			{
				Renderer::Renderable* arr = debugShapeHandler.addPoint(0, 10.0f, false);
				glm::mat4 tempTransform(glm::translate(3.0f,3.0f,10.0f));
				arr->addUniform(Engine::Renderer::UniformFormat::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", 64, &tempTransform);
				renderer->repaint();
			}

			if ( (GetAsyncKeyState(0x4C) & 0x8000 ) && debugShapeHandler.enoughTimeElapsedSinceLastAdd() )
			{
				Renderer::Renderable* arr = debugShapeHandler.addLine(0, 10.0f, false);
				glm::mat4 tempTransform(glm::translate(renderer->camera.facing * 2.0f + renderer->camera.position));
				arr->addUniform(Engine::Renderer::UniformFormat::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", 64, &tempTransform);
				renderer->repaint();
			}

			if ( (GetAsyncKeyState(0x4D) & 0x8000 ) && debugShapeHandler.enoughTimeElapsedSinceLastAdd() )
			{
				Renderer::Renderable* arr = debugShapeHandler.addCube(0, 10.0f, true);
				glm::mat4 tempTransform(glm::translate(renderer->camera.facing * 2.0f + renderer->camera.position));
				arr->addUniform(Engine::Renderer::UniformFormat::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", 64, &tempTransform);
				renderer->repaint();
			}

			if ( (GetAsyncKeyState(0x4E) & 0x8000 ) && debugShapeHandler.enoughTimeElapsedSinceLastAdd() )
			{
				Renderer::Renderable* arr = debugShapeHandler.addBoundingBox(0, 2, 10.0f, true);
				glm::mat4 tempTransform(glm::translate(6.0f,0.0f,0.0f));
				arr->addUniform(Engine::Renderer::UniformFormat::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", 64, &tempTransform);
				renderer->repaint();
			}

			if ( (GetAsyncKeyState(0x4F) & 0x8000 ) && debugShapeHandler.enoughTimeElapsedSinceLastAdd() )
			{
				Renderer::Renderable* arr = debugShapeHandler.addBoundingBox(0, 2, 10.0f, true);
				glm::mat4 tempTransform(glm::translate(0.0f,3.0f,10.0f) * glm::rotate(40.0f, 1.0f, 0.0f, 0.0f));
				arr->addUniform(Engine::Renderer::UniformFormat::GLMMAT4_AFFECTEDBYVIEWPERSPECTIVE, "transform", 64, &tempTransform);
				renderer->repaint();
			}

			if ( (GetAsyncKeyState(0x50) & 0x8000 ))
			{
				DebugMenu::getInstance().setCameraToRendererCamera();
			}

			if (!firstArr)
			{
				xformMat = glm::translate(deltaTime, 0.0f, 0.0f) * xformMat;
				arrowTransform->updateInfo(64, &xformMat[0][0]);
				renderer->repaint();
			}
			renderer->update();
			DebugMenu::getInstance().update();
			renderer->repaint();
		}

	#endif
	#pragma endregion
}