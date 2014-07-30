#pragma once
#include <Qt\qwidget.h>
#include "VerletSystem.h"
#include "ExportHeader.h"

namespace Engine
{
	class ENGINE_SHARED VerletCanvas2D : public QWidget
	{
		int numColors;
		int selectedPoint;
		glm::vec2 selectedPos;
		QColor colors[5];
		float screenScale;
		
		float lastX;
		float lastY;

		float xOff;
		float yOff;

	public:
		float pixelsPerUnit;

		VerletSystem vSystem;
		VerletCanvas2D();
		~VerletCanvas2D();

		void frame();

	protected:
		void paintEvent(QPaintEvent* e);
		void mouseMoveEvent(QMouseEvent *e);
		void mousePressEvent(QMouseEvent *e);
		void wheelEvent(QWheelEvent* e);
	};
}