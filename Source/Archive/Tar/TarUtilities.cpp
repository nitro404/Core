#include "TarUtilities.h"

#include <spdlog/spdlog.h>

uint64_t TarUtilities::parseOctalNumber(const std::string & data) {
	if(data.empty()) {
		return 0;
	}

	size_t i = data.length();
	uint64_t value = 0;
	uint64_t multiplier = 1;

	do {
		i--;

		if(!std::isdigit(data[i])) {
			continue;
		}

		value += (static_cast<uint64_t>(data[i]) - 48) * multiplier;
		multiplier *= 8;
	} while(i != 0);

	return value;
}
