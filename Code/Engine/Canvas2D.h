#pragma once
#include <Qt\qwidget.h>
#include "ExportHeader.h"
#include "Particle2D.h"
#include "Particle2DImpulseRegistry.h"
#include "Particle2DContactResolver.h"

namespace Engine
{
	class PIG2D_Anchored;
	class PIG2D_Bungee;
	class PIG2D_Drag;
	class PIG2D_Spring;

	class ENGINE_SHARED Canvas2D : public QWidget
	{
		int numColors;
		QColor colors[5];
		float screenScale;

	public:
		float pixelsPerUnit;

		struct VectorDrawInfo
		{
			glm::vec2* origin;
			glm::vec2* direction;
			unsigned int color;
			bool active;
			void initialize(glm::vec2* origin, glm::vec2* direction, unsigned int colorIndex, bool active = true);
		} vectorDrawInfos[100];
		int nextVectorDrawInfo;

		int nextParticle;
		Particle2D particles[1000];
		Particle2D* makeParticle();
		Particle2DContactResolver resolver;

		int nextPig;
		Particle2DImpulseGenerator* pigs[100];

		Particle2DImpulseRegistry registry;
		
		float lastX;
		float lastY;

		float xOff;
		float yOff;
		
		Canvas2D();
		~Canvas2D();
		void frame();

		//PIG Factory Functions
		int addPIG_Anchor(Particle2D* p, float anchorX, float anchorY, float springConstant, float restLength);
		
		int* addPIG_Bungee2Particles(Particle2D* p1, Particle2D* p2, float springConstant, float restLength);
		//int addPIG_Bungee(int particleIndex, float anchorX, float anchorY, float springConstant, float restLength);
		
		int addPIG_Drag(Particle2D* p, float k1, float k2);
		
		int* addPIG_Spring2Way(Particle2D* p1, Particle2D* p2, float springConstant, float restLength);
		int addPIG_Spring1Way(Particle2D* lead, Particle2D* follow, float springConstant, float restLength);

		int addPIG_Buoyancy(Particle2D* p, float maxDepth, float volume, float waterHeight, float liquidDensity);

		void removePIG(Particle2D* particle, Particle2DImpulseGenerator* pig);
		void clearPIGs();
		
		int addVectorToDraw(glm::vec2* origin, glm::vec2* vectorToDraw, unsigned int colorIndex, bool active = true);
		void setVectorDrawInfoActive(int index, bool active);


		bool autoStepping;
		bool stepNextFrame;
			void runNextFrame();

		bool vectorsVisible;

	protected:
		virtual void updateTick();

		void paintEvent(QPaintEvent* e);
		void mouseMoveEvent(QMouseEvent *e);
		void mousePressEvent(QMouseEvent *e);
		void wheelEvent(QWheelEvent* e);

		bool forcePause;
	};
}
