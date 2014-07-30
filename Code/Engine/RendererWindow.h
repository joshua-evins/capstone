#pragma once
#include <Qt\qmainwindow.h>
#include "ExportHeader.h"
#include "Clock.h"


namespace Engine
{
	class Renderer;
	class ENGINE_SHARED RendererWindow : public QMainWindow
	{
	protected:
		float elapsed;
		int numFrames;
		Clock clock;
		bool initialized;
		void timerEvent( QTimerEvent* event);
	public:
		RendererWindow();

		Renderer* renderer;

		virtual void initialize();
		virtual void inheritedUpdate();
	};
}