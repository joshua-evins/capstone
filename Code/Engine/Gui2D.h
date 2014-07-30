#pragma once
#include "Qt\qwidget.h"
#include "ExportHeader.h"
#include <Qt\qobject.h>
#include "SliderInfo.h"
#include <FastDelegate.h>
#include "ButtonInfo.h"

namespace Neumont { class Slider; }

class QTimer;
class QVBoxLayout;
class QHBoxLayout;
class QCheckBox;
class QPushButton;

namespace Engine
{
	class Canvas2D;
	class VerletCanvas2D;

	typedef fastdelegate::FastDelegate0<void> DoubleVoid;
	typedef fastdelegate::FastDelegate1<float, void> float_mutator;

	class ENGINE_SHARED Gui2D : public QWidget
	{
		Q_OBJECT

		struct CheckboxInfo
		{
			QCheckBox* checkbox;
			bool* updateTarget;
		};

		struct SliderInfo
		{
			Neumont::Slider* slider;
			float* updateTarget;
		};

		struct SliderMutatorInfo
		{
			Neumont::Slider* slider;
			float_mutator callback;
		};

	public:
		QTimer* updateTimer;
		Canvas2D* drawTarget;
		VerletCanvas2D* verletDrawTarget;
		QVBoxLayout* mainLayout;
		
		QVBoxLayout* sliderLayout;
			QHBoxLayout* currentWidgetRow;

		QHBoxLayout* drawLayout;

		int nextSliderInfo;
		SliderInfo sliderInfos[36];

		int nextCheckboxInfo;
		CheckboxInfo checkboxInfos[10];

		int nextSliderMutatorInfo;
		SliderMutatorInfo sliderMutatorInfos[21];

		int nextButtonInfo;
		ButtonInfo buttonInfos[8];

		void addSliderControl(const char* label, float* targetValue, float initialValue, float min = -10.0f, float max = 10.0f); // pass a new slider and tie its value to the float*
		void addSliderControlForMutator(const char* label, float_mutator callback, float initialValue, float min = -10.0f, float max = 10.0f); // pass a new slider and tie its value to the float*
		void nextWidgetRow(); // switch currentWidgetrRow

		void addButtonControl(const char* label, DoubleVoid callback);

		void addCheckboxControl(const char* label, bool* targetValue);

		void setCanvas2D(Canvas2D* canvas);

		Gui2D(Canvas2D* canvas);
		Gui2D(VerletCanvas2D* canvas);
		~Gui2D();
public slots:
		void frame();
public:
		int x;
	protected:
		void paintEvent(QPaintEvent* e);
	};
}