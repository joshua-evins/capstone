#include "DebugMenuManager.h"
#include <QtGui\qmainwindow.h>
#ifdef ENGINE_DEBUG
#include "DebugMenu.h"
#endif
namespace Engine
{
	DebugMenuManager DebugMenuManager::_instance;

	void DebugMenuManager::attachDebugMenu(QMainWindow* window)
	{
#ifdef ENGINE_DEBUG
		menus[window] = new DebugMenu();
		window->addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, menus[window]);
#endif
	}

	void DebugMenuManager::controlBool(QMainWindow* window, const char* description, bool* theBool)
	{
#ifdef ENGINE_DEBUG
		menus[window]->controlBool(description, theBool);
#endif
	}

	void DebugMenuManager::slideFloat(QMainWindow* window, const char* description, float* theFloat)
	{
#ifdef ENGINE_DEBUG
		menus[window]->slideFloat(description, theFloat);
#endif
	}

	void DebugMenuManager::slideFloat(QMainWindow* window, const char* description, float* theFloat, float min, float max)
	{
#ifdef ENGINE_DEBUG
		menus[window]->slideFloat(description, theFloat, min, max);
#endif
	}

	void DebugMenuManager::watchFloat(QMainWindow* window, const char* description, const float* theFloat)
	{
#ifdef ENGINE_DEBUG
		menus[window]->watchFloat(description, theFloat);
#endif
	}
}