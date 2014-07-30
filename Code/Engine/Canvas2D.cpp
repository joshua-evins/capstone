#include "Canvas2D.h"
#include <Qt\qpainter.h>
#include <iostream>
#include <Qt\qevent.h>
#include "PIG2D_Anchored.h"
#include "PIG2D_Bungee.h"
#include "PIG2D_Drag.h"
#include "PIG2D_Spring.h"
#include "PIG2D_Buoyancy.h"
#include "Particle2DContact.h"


namespace Engine
{

	void Canvas2D::VectorDrawInfo::initialize(glm::vec2* origin, glm::vec2* direction, unsigned int colorIndex, bool active)
	{
		this->origin = origin;
		this->direction = direction;
		this->color = colorIndex;
		this->active = active;
	}

	int Canvas2D::addVectorToDraw(glm::vec2* origin, glm::vec2* vectorToDraw, unsigned int colorIndex, bool active)
	{
		colorIndex = colorIndex%5;
		vectorDrawInfos[nextVectorDrawInfo].initialize(origin, vectorToDraw, colorIndex, active);
		
		return nextVectorDrawInfo++;
	}

	void Canvas2D::setVectorDrawInfoActive(int index, bool active)
	{
		vectorDrawInfos[index].active = active;
	}

	#pragma region Constructors and Destructors
		Canvas2D::Canvas2D() : resolver(1)
		{
			autoStepping = false;
			stepNextFrame = false;
			vectorsVisible = true;
			forcePause = false;
			
			screenScale = 10.0f;
			numColors = 5;
				colors[0] = QColor(255,0,0);
				colors[1] = QColor(0,255,0);
				colors[2] = QColor(0,0,255);
				colors[3] = QColor(0,127,127);
				colors[4] = QColor(127,0,127);

			nextParticle = 0;
			nextPig = 0;
			nextVectorDrawInfo = 0;
			xOff = 100.0f;
			yOff = 100.0f;
			pixelsPerUnit = 1.0f;
		}

		Canvas2D::~Canvas2D()
		{
			// TODO: cleanup memory
		}
	#pragma endregion

	Particle2D* Canvas2D::makeParticle()
	{
		if(nextParticle == 100)
			std::cout << "Too many particles in Canvas2D" << std::endl;

		return particles + nextParticle++;
	}

	#pragma region Update
		void Canvas2D::frame()
		{
			updateTick();

			//update dt
			if((!forcePause) && (autoStepping || stepNextFrame))
			{
				for(int i=0; i<nextParticle; i++)
					particles[i].integrate(0.025f);

				
				
				stepNextFrame = false;

			}
			
			repaint();
		}

		void Canvas2D::updateTick()
		{
			// override function for inheriting classes
		}
	#pragma endregion

	#pragma region Draw
		void Canvas2D::paintEvent(QPaintEvent* /* e */)
		{
			// draw function
			QPainter painter(this);
			painter.translate(width()/2 + xOff, height()/2 + yOff);
			painter.scale(pixelsPerUnit, -pixelsPerUnit);

			int boxDimension = 10000;
			int gridLines = 100;
			for(int i=0; i<gridLines; i++)
			{
				painter.setPen(QPen(Qt::black, 1));
				int lineOffset = boxDimension/gridLines * (i-gridLines/2);
				if(i==gridLines/2)
					painter.setPen(QPen(Qt::gray, 1));
				painter.drawLine(lineOffset, -boxDimension/2, lineOffset, boxDimension/2);
				painter.drawLine(-boxDimension/2, lineOffset, boxDimension/2, lineOffset);
			}

			for(int i=0; i<nextParticle; i++)
			{
				float brushSize = particles[i].getMassRadius();
				painter.setPen(QPen(colors[i%numColors], brushSize));
				painter.drawEllipse(particles[i].position.x * screenScale - brushSize/2, particles[i].position.y * screenScale - brushSize/2, brushSize, brushSize);
				//painter.drawPoint(particles[i].position.x, particles[i].position.y);
			}

			if(vectorsVisible)
				{
					for(int v=0;v<nextVectorDrawInfo;v++)
					{
						if(vectorDrawInfos[v].active)
						{
							VectorDrawInfo& info = vectorDrawInfos[v];
						#pragma region CopyPasta 
							painter.setPen(
								QPen(QBrush(colors[info.color]), 3));
							// Draw the main arrow body:
							QPointF actualBase(info.origin->x, info.origin->y);
							QPointF actualTip(info.direction->x, info.direction->y);
							actualTip += actualBase;

							actualBase *= screenScale;
							actualTip *= screenScale;

							painter.drawLine(actualBase, actualTip);
							// Draw the arrow tip
							float baseAngle = atan2((float)actualTip.y() - actualBase.y(),
							(float)actualTip.x() - actualBase.x());

							const float EDGE_LENGTH = 15; // Pixels
							const float PI = 3.1415927f;
							const float OFFSET_ANGLE = 3 * (PI / 4);

							// The tip is the pivot point for the edges
							float leftEdgeAngle = baseAngle + OFFSET_ANGLE;
							painter.drawLine(actualTip,
							QPoint(actualTip.x() + cos(leftEdgeAngle) * EDGE_LENGTH,
							actualTip.y() + sin(leftEdgeAngle) * EDGE_LENGTH));

							float rightEdgeAngle = baseAngle - OFFSET_ANGLE;
							painter.drawLine(actualTip,
							QPoint(actualTip.x() + cos(rightEdgeAngle) * EDGE_LENGTH,
							actualTip.y() + sin(rightEdgeAngle) * EDGE_LENGTH));
						#pragma endregion
						}
					}
				}
		}
	#pragma endregion

	#pragma region Factory Functions

		int Canvas2D::addPIG_Anchor(Particle2D* p, float anchorX, float anchorY, float springConstant, float restLength)
		{
			pigs[nextPig] = new PIG2D_Anchored(new glm::vec2(anchorX, anchorY), springConstant, restLength);
			registry.add(p, pigs[nextPig]);
			return nextPig++;
		}
		
		int* Canvas2D::addPIG_Bungee2Particles(Particle2D* p1, Particle2D* p2, float springConstant, float restLength)
		{
			pigs[nextPig] = new PIG2D_Bungee(p2, springConstant, restLength);
			pigs[nextPig+1] = new PIG2D_Bungee(p1, springConstant, restLength);

			registry.add(p1, pigs[nextPig]);
			registry.add(p2, pigs[nextPig+1]);

			int retInts[2];
			retInts[0] = nextPig;
			retInts[1] = nextPig+1;

			nextPig += 2;

			return retInts;
		}

		/*int Canvas2D::addPIG_Bungee(int particleIndex, float anchorX, float anchorY, float springConstant, float restLength)
		{
			pigs[nextPig] = new PIG2D_Bungee(particles + particleIndex, springConstant, restLength);
			registry.add(particles + particleIndex, pigs[nextPig]);
			return nextPig++;
		}*/
		
		int Canvas2D::addPIG_Drag(Particle2D* p, float k1, float k2)
		{
			pigs[nextPig] = new PIG2D_Drag(k1, k2);
			registry.add(p, pigs[nextPig]);
			return nextPig++;
		}
		
		int* Canvas2D::addPIG_Spring2Way(Particle2D* p1, Particle2D* p2, float springConstant, float restLength)
		{
			pigs[nextPig] = new PIG2D_Spring(p2, springConstant, restLength);
			pigs[nextPig+1] = new PIG2D_Spring(p1, springConstant, restLength);
			registry.add(p1, pigs[nextPig]);
			registry.add(p2, pigs[nextPig + 1]);
			
			int retInts[2];
			retInts[0] = nextPig;
			retInts[1] = nextPig+1;

			nextPig += 2;

			return retInts;
		}

		int Canvas2D::addPIG_Spring1Way(Particle2D* lead, Particle2D* follow, float springConstant, float restLength)
		{
			pigs[nextPig] = new PIG2D_Spring(lead, springConstant, restLength);
			registry.add(follow, pigs[nextPig]);
			
			return nextPig++;
		}

		int Canvas2D::addPIG_Buoyancy(Particle2D* p, float maxDepth, float volume, float waterHeight, float liquidDensity)
		{
			pigs[nextPig] = new PIG2D_Buoyancy(maxDepth, volume, waterHeight, liquidDensity);
			registry.add(p, pigs[nextPig]);
			
			return nextPig++;
		}

	#pragma endregion

		void Canvas2D::removePIG(Particle2D* particle, Particle2DImpulseGenerator* pig)
		{
			registry.remove(particle, pig);
		}

		void Canvas2D::clearPIGs()
		{
			registry.clear();
		}

		void Canvas2D::runNextFrame()
		{
			forcePause = false;
			stepNextFrame = true;
		}

	#pragma region Mouse Events
		void Canvas2D::mouseMoveEvent(QMouseEvent *e)
		{
			float dx = e->x() - lastX;
			float dy = e->y() - lastY;
			lastX = e->x();
			lastY = e->y();

			xOff += dx;
			yOff += dy;
		}

		void Canvas2D::mousePressEvent(QMouseEvent *e)
		{
			lastX = e->x();
			lastY = e->y();
		}

		void Canvas2D::wheelEvent(QWheelEvent* e)
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
	#pragma endregion
}