#include "Circle.h"

Circle::Circle(float x, float y, int diameterIndex)
{
	diameter = diameters[diameterIndex];
	center = glm::vec2(x, y);

	if(diameterIndex <= 4)
	{
		left = new Circle(center.x - diameter/2 + radii[diameterIndex+2], center.y, diameterIndex+2);
		right = new Circle(center.x - diameter/2 + diameters[diameterIndex+2] + radii[diameterIndex+1], center.y, diameterIndex+1);
	}
	else
	{
		left = NULL;
		right = NULL;
	}

	top = NULL;
	bottom = NULL;
}

Circle::Circle(float x, float y, float diameter)
{
	this->diameter = diameter;
	center = glm::vec2(x, y);

	if(diameter > 64)
	{
		left = new Circle(center.x - diameter/4.0f, center.y, diameter/2.0f);
		right = new Circle(center.x + diameter/4.0f, center.y, diameter/2.0f);
		float yoff = diameter/8.0f;
		top = new Circle(center.x, center.y + diameter/2.0f - yoff, diameter/4.0f);
		bottom = new Circle(center.x, center.y - diameter/2.0f + yoff, diameter/4.0f);
	}
	else
	{
		left = NULL;
		right = NULL;
		top = NULL;
		bottom = NULL;
	}
}

float Circle::diameters[11];
float Circle::radii[11];