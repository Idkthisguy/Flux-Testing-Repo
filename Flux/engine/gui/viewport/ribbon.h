#pragma once
#include <string>
#include <vector>

namespace Flux {
	class Ribbon {
		public:
			void renderRibbon();
		private:
			void drawFileMenu();
			void drawEditMenu();
			void drawProjectControls();
	};
}