#include "DebugMenu.h"
#ifdef ENGINE_DEBUG
#include <QtGui\qboxlayout>
#include <QtGui\qlabel.h>
#include "Slider.h"
#include <QtGui\qcheckbox.h>

namespace Engine
{
	DebugMenu::DebugMenu()
	{
		nextFloatWatcher = 0;
		nextBoolController = 0;
		QWidget* widget;
		setWidget(widget=new QWidget);
		widget->setLayout(mainLayout = new QVBoxLayout);

		startTimer(10);
	}

	void DebugMenu::timerEvent(QTimerEvent*)
	{
		update();
	}

	void DebugMenu::controlBool(const char* description, bool* theBool)
	{
		QHBoxLayout* thisLayout;
		mainLayout->addLayout(thisLayout = new QHBoxLayout);
		thisLayout->addWidget(new QLabel(description));
		BoolControlInfo& w = boolControllers[nextBoolController++];
		thisLayout->addWidget(w.theCheckbox = new QCheckBox);
		w.theBool = theBool;
	}

	void DebugMenu::slideFloat(const char* description, float* theFloat)
	{
		QHBoxLayout* thisLayout;
		mainLayout->addLayout(thisLayout = new QHBoxLayout);
		thisLayout->addWidget(new QLabel(description));
		FloatWatchInfo& w = floatWatchers[nextFloatWatcher++];
		w.slidable = true;
		thisLayout->addWidget(w.theSlider = new Neumont::Slider);
		w.theFloat = theFloat;
	}

	void DebugMenu::slideFloat(const char* description, float* theFloat, float min, float max)
	{
		QHBoxLayout* thisLayout;
		mainLayout->addLayout(thisLayout = new QHBoxLayout);
		thisLayout->addWidget(new QLabel(description));
		FloatWatchInfo& w = floatWatchers[nextFloatWatcher++];
		w.slidable = true;
		thisLayout->addWidget(w.theSlider = new Neumont::Slider(min, max));
		w.theFloat = theFloat;
	}

	void DebugMenu::watchFloat(const char* description, const float* theFloat)
	{
		QHBoxLayout* thisLayout;
		mainLayout->addLayout(thisLayout = new QHBoxLayout);
		thisLayout->addWidget(new QLabel(description));
		FloatWatchInfo& w = floatWatchers[nextFloatWatcher++];
		thisLayout->addWidget(w.theLabel = new QLabel);
		w.theFloat = const_cast<float*>(theFloat);
		w.slidable = false;
	}

	void DebugMenu::update()
	{
		for(unsigned int i=0; i<nextFloatWatcher; i++)
		{
			const FloatWatchInfo&f = floatWatchers[i];
			
			if(f.slidable)
			{
				*(f.theFloat) = f.theSlider->value();
			}
			else
			{
				QString temp;
				temp.setNum(*(f.theFloat));
				f.theLabel->setText(temp);
			}
		}

		for(unsigned int i=0; i<nextBoolController; i++)
		{
			const BoolControlInfo& b = boolControllers[i];
			*(b.theBool) = b.theCheckbox->isChecked();	
		}
	}
}
#endif