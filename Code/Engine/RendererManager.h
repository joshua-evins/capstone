#ifndef RENDERERMANAGER_H
#define RENDERERMANAGER_H
#include "Renderer.h"
#include "DebugShapeHandler.h"
#include "ExportHeader.h"
#include "Clock.h"

class QWidget;
class QVBoxLayout;
class QApplication;

namespace Engine
{
	typedef void (*AddGeometriesCallback)(Renderer* r);
	
	class ENGINE_SHARED RendererManager : public QObject
	{
		Q_OBJECT

	#pragma region Instance Variables
	protected:
		QWidget* mainWindow;
		float deltaTime;
		Clock clock;
		glm::mat4 xformMat;
	public:
		Renderer* renderer;
#ifdef ENGINE_DEBUG
		DebugShapeHandler debugShapeHandler;
#endif
	#pragma endregion

	public:
		void initialize(Renderer* rendererIn);
		virtual void childUpdate();
		void timerEvent( QTimerEvent * event ); // Qt-powered game loop

	#pragma region Debug
	#ifdef ENGINE_DEBUG
		void debugInitialize(QVBoxLayout* mainLayout);
		void debugUpdate(float deltaTime);
	#endif
	#pragma endregion
	};
}

#endif