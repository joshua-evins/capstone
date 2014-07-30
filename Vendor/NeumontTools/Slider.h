#ifndef NEUMONT_SLIDER_H
#define NEUMONT_SLIDER_H
#include <QtGui\qwidget>
#include "ExportImportHeader.h"
class QSlider;
class QLabel;

namespace Neumont
{
	class DLL_SHARED Slider : public QWidget
	{
		Q_OBJECT

		QSlider* slider;
		QLabel* label;

	private slots:
		void sliderValueChanged();

	signals:
		void valueChanged(float newValue);

	public:
		Slider(float min = -10.0f, float max = 10.0f, bool textOnLeft = false);
		float value() const;
		void setValue(float newValue);
	};
}
#endif