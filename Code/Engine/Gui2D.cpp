#include "Gui2D.h"
#include <Qt\qpainter.h>
#include <iostream>
#include <Qt\qtimer.h>
#include <QtGui\qvboxlayout>
#include <QtGui\qhboxlayout>
#include <QtGui\qslider.h>
#include <Slider.h>
#include <QtGui\qpushbutton.h>
#include <QtGui\qcheckbox.h>
#include <Qt\qlabel.h>
#include "Canvas2D.h"
#include "VerletCanvas2D.h"

namespace Engine
{
	#pragma region Constructors and Destructors
		Gui2D::Gui2D(Canvas2D* canvas)
		{
			verletDrawTarget = 0x0;
			x = 0;
			nextSliderInfo = 0;
			nextSliderMutatorInfo = 0;
			nextCheckboxInfo = 0;
			nextButtonInfo = 0;

			updateTimer = new QTimer(this);
			connect(updateTimer, SIGNAL(timeout()), this, SLOT(frame()));
			updateTimer->start(25); //40fps

			setLayout(mainLayout = new QVBoxLayout());
			mainLayout->addLayout(sliderLayout = new QVBoxLayout());
			mainLayout->addLayout(drawLayout = new QHBoxLayout());
			//sliderLayout->addWidget(xSlider = new Neumont::Slider(0, 300.0f));
			drawTarget = canvas;
			drawLayout->addWidget(drawTarget);
			drawTarget->show();
			sliderLayout->addLayout(currentWidgetRow = new QHBoxLayout());
			resize(800, 600);
			drawTarget->setMinimumHeight(800);
			drawTarget->setMinimumWidth(1200);
		}

		Gui2D::Gui2D(VerletCanvas2D* canvas)
		{
			drawTarget = 0x0;
			x = 0;
			nextSliderInfo = 0;
			nextSliderMutatorInfo = 0;
			nextCheckboxInfo = 0;
			nextButtonInfo = 0;

			updateTimer = new QTimer(this);
			connect(updateTimer, SIGNAL(timeout()), this, SLOT(frame()));
			updateTimer->start(25); //40fps

			setLayout(mainLayout = new QVBoxLayout());
			mainLayout->addLayout(sliderLayout = new QVBoxLayout());
			mainLayout->addLayout(drawLayout = new QHBoxLayout());
			//sliderLayout->addWidget(xSlider = new Neumont::Slider(0, 300.0f));
			verletDrawTarget = canvas;
			verletDrawTarget = canvas;
			drawLayout->addWidget(verletDrawTarget);
			verletDrawTarget->show();
			sliderLayout->addLayout(currentWidgetRow = new QHBoxLayout());
			resize(800, 600);
			verletDrawTarget->setMinimumHeight(800);
			verletDrawTarget->setMinimumWidth(1200);
		}

		Gui2D::~Gui2D()
		{
			/*delete mainLayout;
			delete sliderLayout;
			delete drawLayout;
			delete updateTimer;
			delete drawTarget;*/
		}
	#pragma endregion

	void Gui2D::nextWidgetRow()
	{
		sliderLayout->addLayout(currentWidgetRow = new QHBoxLayout());
	}

	void Gui2D::addSliderControl(const char* label, float* targetValue, float initialValue, float min, float max)
	{
		currentWidgetRow->addWidget(new QLabel(label));
		sliderInfos[nextSliderInfo].slider = new Neumont::Slider(min, max);
		sliderInfos[nextSliderInfo].updateTarget = targetValue;
		currentWidgetRow->addWidget( sliderInfos[nextSliderInfo].slider );
		sliderInfos[nextSliderInfo].slider->setValue(initialValue);
		nextSliderInfo++;
	}

	void Gui2D::addSliderControlForMutator(const char* label, float_mutator callback, float initialValue, float min, float max)
	{
		currentWidgetRow->addWidget(new QLabel(label));
		sliderMutatorInfos[nextSliderMutatorInfo].slider = new Neumont::Slider(min, max);
		sliderMutatorInfos[nextSliderMutatorInfo].callback = callback;
		currentWidgetRow->addWidget( sliderMutatorInfos[nextSliderMutatorInfo].slider );
		sliderMutatorInfos[nextSliderMutatorInfo].slider->setValue(initialValue);
		nextSliderMutatorInfo++;
	}

	void Gui2D::addButtonControl(const char* label, DoubleVoid callback)
	{
		ButtonInfo* info = buttonInfos+nextButtonInfo;
		info->callback = callback;
		currentWidgetRow->addWidget(info->button = new QPushButton(label));
		connect(info->button, SIGNAL(released()), info, SLOT(invokeCallback()));
		
		nextButtonInfo++;
	}

	void Gui2D::addCheckboxControl(const char* label, bool* targetValue)
	{
		currentWidgetRow->addWidget(new QLabel(label));
		checkboxInfos[nextCheckboxInfo].checkbox = new QCheckBox();
		checkboxInfos[nextCheckboxInfo].updateTarget = targetValue;
		currentWidgetRow->addWidget( checkboxInfos[nextCheckboxInfo].checkbox );
		checkboxInfos[nextCheckboxInfo].checkbox->setChecked(*targetValue);
		nextCheckboxInfo++;
	}

	void Gui2D::paintEvent(QPaintEvent* /* e */)
	{
	}

	void Gui2D::frame()
	{	
		// update function
		for(int i=0; i<nextSliderInfo; i++)
		{
			*(sliderInfos[i].updateTarget) = sliderInfos[i].slider->value();
		}

		for(int i=0; i<nextCheckboxInfo; i++)
		{
			*(checkboxInfos[i].updateTarget) = checkboxInfos[i].checkbox->isChecked();
		}

		for(int i=0; i<nextSliderMutatorInfo; i++)
		{
			sliderMutatorInfos[i].callback(sliderMutatorInfos[i].slider->value());
		}

		if(drawTarget)
			drawTarget->frame();
		if(verletDrawTarget)
			verletDrawTarget->frame();
	}
}