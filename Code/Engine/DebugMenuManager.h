#pragma once
#include "ExportHeader.h"
#include "typedefs_Engine.h"
#ifdef ENGINE_DEBUG
#include <map>
#endif

class QMainWindow;

namespace Engine
{
#ifdef ENGINE_DEBUG
	class DebugMenu;
#endif

	class ENGINE_SHARED DebugMenuManager
	{
		static DebugMenuManager _instance;

	private:
		DebugMenuManager() {};
		DebugMenuManager(const DebugMenuManager&);
		DebugMenuManager& operator=(const DebugMenuManager&);

#ifdef ENGINE_DEBUG
		std::map<QMainWindow*, DebugMenu*> menus;
#endif

	public:
		static DebugMenuManager& instance(){return _instance;}
		void attachDebugMenu(QMainWindow* window);

		void controlBool(QMainWindow* window, const char* description, bool* theBool);
		void slideFloat(QMainWindow* window, const char* description, float* theFloat);
		void slideFloat(QMainWindow* window, const char* description, float* theFloat, float min, float max);
		void watchFloat(QMainWindow* window, const char* description, const float* theFloat);
	};
}
