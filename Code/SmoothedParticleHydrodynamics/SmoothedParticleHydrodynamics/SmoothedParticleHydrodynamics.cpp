#include <Qt\qapplication.h>
#include "Window_SPH.h"

int main()
{
	int x=0;
	QApplication app(x, 0);

	Window_SPH window;
	return app.exec();
}