#pragma once

#include <vector>

namespace RT {

	struct RHIExtensionList {
		std::vector<const char*> Layers;
		std::vector<const char*> Extensions;
	};

}