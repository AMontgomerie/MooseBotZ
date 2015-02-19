#include "StrategyManager.h"

GameState::GameState(int techLevel, int workerCount, int hatcheryCount, int gasCount)
{
	this->techLevel = techLevel;
	this->workerCount = workerCount;
	this->hatcheryCount = hatcheryCount;
	this->gasCount = gasCount;
}

GameState::GameState()
{
}

int GameState::getTechLevel()
{
	return techLevel;
}

int GameState::getWorkerCount()
{
	return workerCount;
}

int GameState::getHatcheryCount()
{
	return hatcheryCount;
}

int GameState::getGasCount()
{
	return gasCount;
}

void GameState::setTechLevel(int techLevel)
{
	this->techLevel = techLevel;
}

void GameState::setWorkerCount(int workerCount)
{
	this->workerCount = workerCount;
}

void GameState::setHatcheryCount(int hatcheryCount)
{
	this->hatcheryCount = hatcheryCount;
}

void GameState::setGasCount(int gasCount)
{
	this->gasCount = gasCount;
}

StrategyManager::StrategyManager() 
{
	initialState.setTechLevel(1);
	initialState.setWorkerCount(4);
	initialState.setHatcheryCount(1);
	initialState.setGasCount(0);

	currentState.setTechLevel(1);
	currentState.setWorkerCount(4);
	currentState.setHatcheryCount(1);
	currentState.setGasCount(0);

	earlyGame.setTechLevel(1);
	earlyGame.setWorkerCount(16);
	earlyGame.setHatcheryCount(3);
	earlyGame.setGasCount(1);

	midGame.setTechLevel(2);
	midGame.setWorkerCount(24);
	midGame.setHatcheryCount(4);
	midGame.setGasCount(2);

	lateGame.setTechLevel(3);
	lateGame.setWorkerCount(36);
	lateGame.setHatcheryCount(5);
	lateGame.setGasCount(2);

	nextState = &earlyGame;

	lair = false;
	hive = false;
}

void StrategyManager::changeState()
{
	if(nextState == &earlyGame)
	{
		nextState = &midGame;
	}
	if(nextState == &midGame)
	{
		nextState = &lateGame;
	}
}

std::vector<std::pair<MetaType, int>> StrategyManager::getNewGoal()
{
	std::vector<std::pair<MetaType, int>> newGoal;

	if((currentState.getWorkerCount() < nextState->getWorkerCount()) && (armyStatus != defend) && !threatStatus)
	{
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Drone, (nextState->getWorkerCount() - currentState.getWorkerCount())));
	}
	if((currentState.getHatcheryCount() < nextState->getHatcheryCount()) && (armyStatus != defend)  && !threatStatus)
	{
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Hatchery, 1));
	}
	if(currentState.getGasCount() < nextState->getGasCount() && (armyStatus != defend) && !threatStatus)
	{
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Extractor, 1));
	}

	if((currentState.getWorkerCount() >= nextState->getWorkerCount()) &&
		(currentState.getHatcheryCount() >= nextState->getHatcheryCount()) &&
		(currentState.getGasCount() >= nextState->getGasCount()))
	{
		if((nextState->getTechLevel() == 2) && !lair)
		{
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Lair, 1));
		}
		if((nextState->getTechLevel() == 3) && lair && !hive)
		{
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Hive, 1));
		}
		changeState();
	}

	if((currentState.getTechLevel() == 2) && !lair)
	{
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Lair, 1));
	}
	if((currentState.getTechLevel() == 3) && lair && !hive)
	{
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Hive, 1));
	}

	newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Zergling, 3));
	//newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Hydralisk, 4));

	switch(currentState.getTechLevel())
	{
	case 1:
		if(threatStatus)
		{
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Sunken_Colony, 1));
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Zergling, 2));

		}
		break;
	case 2:
		if(threatStatus)
		{
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Sunken_Colony, 1));
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Zergling, 3));
		}
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Mutalisk, 4));
		break;
	case 3:
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Zergling, 4));
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Ultralisk, 3));
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Mutalisk, 1));
	}

	return newGoal;
}

void StrategyManager::update(int techLevel, int armyStatus)
{
	int hatcheryCount = 0;
	int workerCount = 0;
	int gasCount = 0;
	this->armyStatus = armyStatus;

	lair = false;
	hive = false;

	for(std::set<BWAPI::Unit*>::const_iterator i = BWAPI::Broodwar->self()->getUnits().begin(); i != BWAPI::Broodwar->self()->getUnits().end(); i++)
	{
		if((*i)->getType().isResourceDepot() || (*i)->getBuildType().isResourceDepot())
		{
			hatcheryCount++;
		}
		else if((*i)->getType().isWorker()  || (*i)->getBuildType().isWorker())
		{
			workerCount++;
		}
		else if((*i)->getType().isRefinery()  || (*i)->getBuildType().isRefinery())
		{
			gasCount++;
		}
		if(((*i)->getType() == BWAPI::UnitTypes::Zerg_Lair) || ((*i)->getBuildType() == BWAPI::UnitTypes::Zerg_Lair))
		{
			lair = true;
			//changeState();
		}
		if(((*i)->getType() == BWAPI::UnitTypes::Zerg_Hive) || ((*i)->getBuildType() == BWAPI::UnitTypes::Zerg_Hive))
		{
			hive = true;
			//changeState();
		}
		if((*i)->getType() == BWAPI::UnitTypes::Zerg_Lair)
		{
			techLevel = 2;
		}
		if((*i)->getType() == BWAPI::UnitTypes::Zerg_Hive)
		{
			techLevel = 3;
		}
	}

	currentState.setTechLevel(techLevel);
	currentState.setWorkerCount(workerCount);
	currentState.setHatcheryCount(hatcheryCount);
	currentState.setGasCount(gasCount);
}

void StrategyManager::setThreatStatus(bool threat)
{
	if(threat)
	{
		threatStatus = true;
	}
	else
	{
		threatStatus = false;
	}
}