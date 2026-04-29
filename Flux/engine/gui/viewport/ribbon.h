#pragma once
#include <string>
#include <vector>



namespace Flux {
	enum ToolMode {
		TOOL_MOVE = 0,
		TOOL_ROTATE = 1,
		TOOL_SCALE = 2
	};

	class Ribbon {
		public:
			void renderRibbon();
		private:
			void drawFileMenu();
			void drawEditMenu();
			void drawProjectControls();
			void drawTransformTools();
	};
}