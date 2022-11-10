#pragma once

class StatisticsCollector
{
private:
	void ramUsage();
	void vramUsage();

public:
	StatisticsCollector();
	~StatisticsCollector();

	void update();
};