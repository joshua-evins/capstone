#ifndef ENGINE_Camera_H
#define ENGINE_Camera_H
#include <glm\glm.hpp>
#include "typedefs_Engine.h"
#include "ExportHeader.h"


namespace Engine
{
	class ENGINE_SHARED Camera
	{
		public:

	#pragma region Instance Variables
		glm::vec3 position;
		glm::vec3 facing;
		glm::vec3 up;

		float MovementMultiplier;
		float TurningMultiplier;
	#pragma endregion

	#pragma region Constructors and Initializers
		Camera();
		void Camera::initialize(const glm::vec3& positionIn, const glm::vec3& facingIn, const glm::vec3& upIn);
	#pragma endregion

	#pragma region Movement Functions

		void moveForward();

		void moveBackward();
	
		void moveLeft();

		void moveRight();

		void moveUp();

		void moveDown();

		// increases or decreases movement speed
		void stepMovement(float i);

		void turn(int pixelsMovedHorizontal, int pixelsMovedVertical);
	#pragma endregion
		
		bool update(int pixelsMovedHorizontal, int pixelsMovedVertical);
		glm::mat4 getViewMatrix() const;

	#pragma region Debug
#ifdef ENGINE_DEBUG
		//TODO: remove this and just use position
			glm::vec3* debug_getCameraPosition();
#endif
	#pragma endregion

	};
}
#endif