#pragma once
#include <string>
#include <ctime>

struct Metric {
	time_t timestamp;
	std::string processName;
	std::string windowTitle;
	bool userActive;
};