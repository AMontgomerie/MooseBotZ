#pragma once
#include "MetaType.h"

class GameState
{
private:
	int techLevel;
	int workerCount;
	int hatcheryCount;
	int gasCount;
public:
	GameState();
	GameState(int techLevel, int workerCount, int hatcheryCount, int gasCount);
	~GameState() {};
	int getTechLevel();
	int getWorkerCount();
	int getHatcheryCount();
	int getGasCount();
	void setTechLevel(int techLevel);
	void setWorkerCount(int workerCount);
	void setHatcheryCount(int hatcheryCount);
	void setGasCount(int gasCount);
};

class StrategyManager 
{
	GameState currentState;
	GameState initialState;
	GameState earlyGame;
	GameState midGame;
	GameState lateGame;
	GameState *nextState;
	bool lair;
	bool hive;
	int armyStatus;
public:	
	StrategyManager();
	~StrategyManager() {};
	void update(int techLevel, int armyStatus);
	std::vector<std::pair<MetaType, int>> getNewGoal();
	void changeState();
	enum {scout = 0, retreat = 1, attack = 2, defend = 3};
};
