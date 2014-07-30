#include "VerletCanvas2D.h"
#include <Qt\qevent.h>
#include <Qt\qpainter.h>

namespace Engine
{
	VerletCanvas2D::VerletCanvas2D() : vSystem(0.025f)
	{
		numColors = 5;
			colors[0] = QColor(255,0,0);
			colors[1] = QColor(0,255,0);
			colors[2] = QColor(0,0,255);
			colors[3] = QColor(0,127,127);
			colors[4] = QColor(127,0,127);

		selectedPoint = -1;
		screenScale = 10.0f;
		xOff = 0.0f;
		yOff = 0.0f;
		pixelsPerUnit = 1.0f;
	}

	VerletCanvas2D::~VerletCanvas2D()
	{
	}

	void VerletCanvas2D::frame()
	{
		static int call = 0;
		call++;
		//if(call%50 == 40)
			vSystem.TimeStep();
		repaint();
	}

	void VerletCanvas2D::paintEvent(QPaintEvent* )
	{
		QPainter painter(this);
		painter.translate(width()/2 + xOff, height()/2 + yOff);
		painter.scale(pixelsPerUnit, -pixelsPerUnit);

		painter.setPen(QPen(Qt::black, 1));
		for(int i=0; i<vSystem.currentConstraint; i++)
		{
			int p1 = vSystem.constraints[i].p1;
			int p2 = vSystem.constraints[i].p2;

			glm::vec2& pos1 = vSystem.currentPos[p1];
			glm::vec2& pos2 = vSystem.currentPos[p2];

			painter.drawLine(-pos1.x*10.0f, pos1.y*10.0f, -pos2.x*10.0f, pos2.y*10.0f);
		}
		for(int i=0; i<VS_NUMPOINTS; i++)
		{
			if(
				i==1 ||
				i==2 ||
				i==3 ||
				i==40 ||
				i==11 ||
				i==13 ||
				i==21 ||
				i==23 ||
				i==31 ||
				i==33 ||
				i==41 ||
				i==42 ||
				i==43 ||
				i==5 ||
				i==6 ||
				i==7 ||
				i==17 ||
				i==25 ||
				i==26 ||
				i==27 ||
				i==37 ||
				i==45 ||
				i==46 ||
				i==47)
			{
				painter.setPen(QPen(QColor(255, 0, 0), 4));
				glm::vec2 p = vSystem.currentPos[i];
				painter.drawPoint(-p.x*10.0f, p.y*10.0f);
			}
		}
		
		if(selectedPoint > -1)
		{
			painter.setPen(QPen(QColor(0, 255, 0), 10));
			glm::vec2 selPos = vSystem.currentPos[selectedPoint];
			painter.drawPoint(selPos.x*10.0f, selPos.y*10.0f);
		}
	}





	void VerletCanvas2D::mouseMoveEvent(QMouseEvent *e)
	{
		float dx = e->x() - lastX;
			float dy = e->y() - lastY;
			lastX = e->x();
			lastY = e->y();

			//xOff += dx;
			//yOff += dy;

			if(selectedPoint >-1)
			{
				dx = (dx<0.0f) ? -0.08f : 0.08f;
				dy = (dy<0.0f) ? -0.08f : 0.08f;
				vSystem.currentPos[selectedPoint] += glm::vec2(dx, dy);
			}
	}

	void VerletCanvas2D::mousePressEvent(QMouseEvent *e)
	{
		lastX = e->x();
		lastY = e->y();
		int relX = e->x() - width()/2 + xOff;
		int relY = e->y() - height()/2 + yOff;
		selectedPoint = vSystem.closestParticle(glm::vec2(relX, -relY));
	}

	void VerletCanvas2D::wheelEvent(QWheelEvent* e)
	{
		if(e->delta() > 0 && pixelsPerUnit < 4.0f)
			{
				pixelsPerUnit += 0.1f;
			}
			else if(e->delta() < 0 && pixelsPerUnit > 0.2f)
			{
				pixelsPerUnit -= 0.1f;
			}
	}
}