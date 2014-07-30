#include "Slider.h"
#include <QtGui\qvboxlayout>
#include <QtGui\qslider>
#include <QtGui\qlabel>

namespace Neumont
{
	static const float SLIDER_GRANULARITY = 20.0;

	Slider::Slider(float min, float max, bool textOnLeft)
	{
		QLayout* layout;
		setLayout(layout = textOnLeft ? (QLayout*)new QHBoxLayout : new QVBoxLayout);
		layout->addWidget(label = new QLabel);
		label->setMinimumWidth(35);
		layout->addWidget(slider = new QSlider);
		label->setAlignment(Qt::AlignCenter);
		slider->setOrientation(Qt::Horizontal);
		slider->setMinimum(min * SLIDER_GRANULARITY);
		slider->setMaximum(max * SLIDER_GRANULARITY);
		connect(slider, SIGNAL(valueChanged(int)), 
			this, SLOT(sliderValueChanged()));
		sliderValueChanged();
	}

	float Slider::value() const
	{
		return slider->value() / SLIDER_GRANULARITY;
	}

	void Slider::setValue(float newValue)
	{
		slider->setValue(newValue * SLIDER_GRANULARITY);
	}

	void Slider::sliderValueChanged()
	{
		label->setText(QString::number(value()));
		emit valueChanged(value());
	}
}