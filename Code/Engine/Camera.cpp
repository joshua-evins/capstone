#include "Camera.h"
#include <Qt\qevent.h>
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>
#define  WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Engine
{

#pragma region Constructors and Initializers
	Camera::Camera()
	{
		MovementMultiplier = 10.5f;
		TurningMultiplier = 5.0f;
	}

	void Camera::initialize(const glm::vec3& positionIn, const glm::vec3& facingIn, const glm::vec3& upIn)
	{
		MovementMultiplier = 10.5f;
		TurningMultiplier = 5.0f;
		position = positionIn;
		facing = glm::normalize(facingIn);
		up = upIn;
	}
#pragma endregion
	
#pragma region Movement Functions

	bool Camera::update(int pixelsMovedHorizontal, int pixelsMovedVertical)
	{
		if ( GetAsyncKeyState(Qt::Key::Key_W))
		{
			moveForward();
		}
		if ( GetAsyncKeyState(Qt::Key::Key_S))
		{
			moveBackward();
		}
		if ( GetAsyncKeyState(Qt::Key::Key_A))
		{
			moveLeft();
		}	
		if ( GetAsyncKeyState(Qt::Key::Key_D))
		{
			moveRight();
		}	
		if ( GetAsyncKeyState(Qt::Key::Key_R))
		{
			moveUp();
		}	
		if ( GetAsyncKeyState(Qt::Key::Key_F))
		{
			moveDown();
		}	
		if ( GetAsyncKeyState(VK_OEM_MINUS))
		{
			stepMovement(-0.2f);
		}	
		if ( GetAsyncKeyState(VK_OEM_PLUS))
		{
			stepMovement(0.2f);
		}
		if ( GetAsyncKeyState(VK_RBUTTON))
		{
			turn(pixelsMovedHorizontal, pixelsMovedVertical);
		}
		return true;
	}

	void Camera::moveForward()
	{
		position += facing / 10.0f * MovementMultiplier;
	}

	void Camera::moveBackward()
	{
		position -= facing / 10.0f * MovementMultiplier;
	}

	void Camera::moveLeft()
	{
		glm::vec3 perp = glm::cross(facing, up);
		perp = glm::normalize(perp);

		glm::vec4 temp(perp, 1.0f);

		temp = temp * glm::rotate(90.0f, perp);
		
		position -= glm::vec3(temp.x, temp.y, temp.z) / 10.0f * MovementMultiplier;
	}

	void Camera::moveRight()
	{
		glm::vec3 perp = glm::cross(facing, up);
		perp = glm::normalize(perp);
		
		glm::vec4 temp(perp, 1.0f);

		temp = temp * glm::rotate(90.0f, perp);
		position += glm::vec3(temp.x, temp.y, temp.z) / 10.0f * MovementMultiplier;
	}

	void Camera::moveUp()
	{
		position += up / 10.0f * MovementMultiplier;
	}

	void Camera::moveDown()
	{
		position -= up / 10.0f * MovementMultiplier;
	}

	void Camera::stepMovement(float i)
	{
		if(MovementMultiplier + i > 1.0f && MovementMultiplier + i < 3.0f)
			MovementMultiplier += i;
	}

	void Camera::turn(int pixelsMovedHorizontal, int pixelsMovedVertical)
	{
		glm::vec4 temp(up, 1.0f);

		//set perp
		glm::vec3 perp = glm::cross(facing, up);
		perp = glm::normalize(perp);
		//////////

		// manage facing vertical turn
		temp = glm::vec4(facing, 1.0f);

		temp = temp * glm::rotate((float)pixelsMovedVertical / TurningMultiplier, perp);
		facing = glm::vec3(temp.x, temp.y, temp.z);
		//////////

		// manage facing horizontal turn
		temp = glm::vec4(facing, 1.0f);

		temp = temp * glm::rotate((float)pixelsMovedHorizontal / TurningMultiplier, up);
		facing = glm::vec3(temp.x, temp.y, temp.z);
		//////////
	}
#pragma endregion

	glm::mat4 Camera::getViewMatrix() const
	{
		return glm::lookAt(position, position+facing, up);
	}

#pragma region Debug
#ifdef ENGINE_DEBUG
	glm::vec3* Camera::debug_getCameraPosition()
	{
		return &position;
	}
#endif
#pragma endregion

}