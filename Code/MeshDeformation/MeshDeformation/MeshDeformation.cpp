#define HEADERSTOP
#include "Window_MD.h"
#include <Qt\qapplication.h>

int main()
{
	int x=0;
	QApplication app(x, 0);

	Window_MD window;
	return app.exec();
}

