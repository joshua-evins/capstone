#pragma once
#include <Qt\qobject.h>
#include <FastDelegate.h>
class QPushButton;

namespace Engine
{
	typedef fastdelegate::FastDelegate0<void> DoubleVoid;

	struct ButtonInfo : public QObject
	{
		Q_OBJECT
	public:
		QPushButton* button;
		DoubleVoid callback;
		public slots:
			void invokeCallback();
	};
}
