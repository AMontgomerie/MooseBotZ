#pragma once
#include "MetaType.h"
#include <string>

class GameState
{
private:
	int techLevel;
	int workerCount;
	int hatcheryCount;
	int gasCount;
	int armyStatus;
	std::string name;
	std::set<BWAPI::UpgradeType> upgrades;
	std::set<BWAPI::UnitType> buildings;
public:
	GameState();
	GameState(int techLevel, int workerCount, int hatcheryCount, int gasCount);
	~GameState() {}
	int getTechLevel();
	int getWorkerCount();
	int getHatcheryCount();
	int getGasCount();
	void setName(std::string name);
	std::string getName();
	void setTechLevel(int techLevel);
	void setWorkerCount(int workerCount);
	void setHatcheryCount(int hatcheryCount);
	void setGasCount(int gasCount);
	void GameState::addRequiredUpgrade(BWAPI::UpgradeType upgrade);
	std::set<BWAPI::UpgradeType> GameState::getRequiredUpgrades();
	void GameState::addRequiredBuilding(BWAPI::UnitType building);
	std::set<BWAPI::UnitType> GameState::getRequiredBuildings();
	void GameState::removeRequiredUpgrade(BWAPI::UpgradeType upgrade);
	void GameState::removeRequiredBuilding(BWAPI::UnitType building);
};

class StrategyManager 
{
	GameState currentState;
	GameState initialState;
	GameState earlyGame;
	GameState midGame;
	GameState lateGame;
	GameState finalState;
	GameState *nextState;
	bool lair;
	bool hive;
	bool spire;
	bool threatStatus;
	int armyStatus;
	int armySupply;
	int sunkenCount;
	std::set<std::pair<BWAPI::UnitType, int>> enemyComposition;
public:	
	StrategyManager();
	~StrategyManager() {}
	static StrategyManager & Instance();
	const void update(int armyStatus);
	const std::vector<std::pair<MetaType, int>> getNewGoal();
	const void changeState();
	const void setThreatStatus(bool threat);
	const void setArmyStatus(int status);
	const void setEnemyComposition(std::set<std::pair<BWAPI::UnitType, int>> composition);
	const void drawEnemyInformation(int x, int y);
	const void setArmySupply(int supply);
	const void addSunken();
	const void removeSunken();
	const void productionStarted(MetaType element);
	const void drawStateInformation(int x, int y);
	const int getState();
	void updateUpgrades(std::vector<BWAPI::UpgradeType> upgrades);
private:
	int calculateEnemyArmySupply();

	enum {scout = 0, retreat = 1, attack = 2, defend = 3};
};
