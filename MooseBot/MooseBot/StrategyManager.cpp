#include "StrategyManager.h"

GameState::GameState(int techLevel, int workerCount, int hatcheryCount, int gasCount)
{
	this->techLevel = techLevel;
	this->workerCount = workerCount;
	this->hatcheryCount = hatcheryCount;
	this->gasCount = gasCount;
	upgrades.clear();
	buildings.clear();
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


void GameState::addRequiredUpgrade(BWAPI::UpgradeType upgrade)
{
	upgrades.insert(upgrade);
}

void GameState::removeRequiredUpgrade(BWAPI::UpgradeType upgrade)
{
	upgrades.erase(upgrade);
}

std::set<BWAPI::UpgradeType> GameState::getRequiredUpgrades()
{
	return upgrades;
}

void GameState::addRequiredBuilding(BWAPI::UnitType building)
{
	buildings.insert(building);
}

void GameState::removeRequiredBuilding(BWAPI::UnitType building)
{
	buildings.erase(building);
}


std::set<BWAPI::UnitType> GameState::getRequiredBuildings()
{
	return buildings;
}

void GameState::setName(std::string name)
{
	this->name = name;
}
std::string GameState::getName()
{
	return name;
}

StrategyManager::StrategyManager() 
{
	initialState.setTechLevel(1);
	initialState.setWorkerCount(4);
	initialState.setHatcheryCount(1);
	initialState.setGasCount(0);
	initialState.setName("Initial State");

	currentState.setTechLevel(1);
	currentState.setWorkerCount(4);
	currentState.setHatcheryCount(1);
	currentState.setGasCount(0);
	currentState.setName("Current State");

	earlyGame.setTechLevel(1);
	earlyGame.setWorkerCount(16);
	earlyGame.setHatcheryCount(3);
	earlyGame.setGasCount(1);
	earlyGame.addRequiredUpgrade(BWAPI::UpgradeTypes::Metabolic_Boost);
	//earlyGame.addRequiredBuilding(BWAPI::UnitTypes::Zerg_Extractor);
	earlyGame.addRequiredBuilding(BWAPI::UnitTypes::Zerg_Lair);
	earlyGame.setName("Early Game");

	midGame.setTechLevel(2);
	midGame.setWorkerCount(24);
	midGame.setHatcheryCount(4);
	midGame.setGasCount(2);
	midGame.addRequiredUpgrade(BWAPI::UpgradeTypes::Pneumatized_Carapace);
	midGame.addRequiredUpgrade(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
	midGame.setName("Mid Game");

	lateGame.setTechLevel(3);
	lateGame.setWorkerCount(36);
	lateGame.setHatcheryCount(6);
	lateGame.setGasCount(2);
	lateGame.addRequiredUpgrade(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
	lateGame.addRequiredUpgrade(BWAPI::UpgradeTypes::Zerg_Carapace);
	lateGame.addRequiredBuilding(BWAPI::UnitTypes::Zerg_Hive);
	lateGame.setName("Late Game");

	finalState.setTechLevel(3);
	finalState.setWorkerCount(50);
	finalState.setHatcheryCount(7);
	finalState.setGasCount(3);
	finalState.addRequiredUpgrade(BWAPI::UpgradeTypes::Adrenal_Glands);
	finalState.addRequiredUpgrade(BWAPI::UpgradeTypes::Chitinous_Plating);
	finalState.addRequiredUpgrade(BWAPI::UpgradeTypes::Zerg_Flyer_Attacks);
	finalState.addRequiredUpgrade(BWAPI::UpgradeTypes::Zerg_Melee_Attacks);
	finalState.addRequiredUpgrade(BWAPI::UpgradeTypes::Zerg_Carapace);
	finalState.setName("Final State");

	nextState = &earlyGame;
	armyStatus = 0;
	sunkenCount = 0;
}

StrategyManager & StrategyManager::Instance()
{
	static StrategyManager instance;

	return instance;
}

const void StrategyManager::changeState()
{
	if(nextState == &earlyGame)
	{
		nextState = &midGame;
		return;
	}
	else if(nextState == &midGame)
	{
		nextState = &lateGame;
		return;
	}
	else if(nextState == &lateGame)
	{
		nextState = &finalState;
		return;
	}
}

const int StrategyManager::getState()
{
	if(nextState == &earlyGame)
	{
		return 0;
	}
	else if(nextState == &midGame)
	{
		return 1;
	}
	else if(nextState == &lateGame)
	{
		return 2;
	}
	else if(nextState == &finalState)
	{
		return 3;
	}
	else
	{
		return 4;
	}
}

const std::vector<std::pair<MetaType, int>> StrategyManager::getNewGoal()
{
	std::vector<std::pair<MetaType, int>> newGoal;

	if((currentState.getWorkerCount() < nextState->getWorkerCount()) && (armyStatus != defend))
	{
		int goalDrones = 0;

		if(!threatStatus)
		{
			if((nextState->getWorkerCount() - currentState.getWorkerCount()) > 8)
			{
				goalDrones = 8;
			}
			else
			{
				goalDrones = nextState->getWorkerCount() - currentState.getWorkerCount();
			}
		}
		else
		{
			if((nextState->getWorkerCount() - currentState.getWorkerCount()) > 2)
			{
				goalDrones = 2;
			}
			else
			{
				goalDrones = nextState->getWorkerCount() - currentState.getWorkerCount();
			}
		}

		if(goalDrones > 0)
		{
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Drone, goalDrones));
		}
	}
	if((currentState.getHatcheryCount() < nextState->getHatcheryCount()) && (armyStatus != defend)  && !threatStatus)
	{
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Hatchery, 1));
	}
	if(currentState.getGasCount() < nextState->getGasCount() && (armyStatus != defend) && !threatStatus)
	{
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Extractor, 1));
	}

	std::vector<std::pair<MetaType, int>> additionalTech;
	
	std::set<BWAPI::UpgradeType> requiredUpgrades = nextState->getRequiredUpgrades();
	std::set<BWAPI::UpgradeType> currentUpgrades = currentState.getRequiredUpgrades();
	for(std::set<BWAPI::UpgradeType>::iterator i = requiredUpgrades.begin(); i != requiredUpgrades.end(); i++)
	{
		bool found = false;
		for(std::set<BWAPI::UpgradeType>::iterator j = currentUpgrades.begin(); j != currentUpgrades.end(); j++)
		{
			if((*i) == (*j))
			{
				found = true;
			}
		}
		if(!found)
		{
			additionalTech.push_back(std::make_pair(MetaType(*i), 1));
		}
	}
	std::set<BWAPI::UnitType> requiredBuildings = nextState->getRequiredBuildings();
	std::set<BWAPI::UnitType> currentBuildings = currentState.getRequiredBuildings();
	for(std::set<BWAPI::UnitType>::iterator i = requiredBuildings.begin(); i != requiredBuildings.end(); i++)
	{			
		bool found = false;
		for(std::set<BWAPI::UnitType>::iterator j = currentBuildings.begin(); j != currentBuildings.end(); j++)
		{
			if((*i) == (*j))
			{
				found = true;
			}
		}
		if(!found)
		{
			additionalTech.push_back(std::make_pair(MetaType(*i), 1));
		}
	}
	
	if(threatStatus && !additionalTech.empty())
	{
		newGoal.push_back(std::make_pair(additionalTech.begin()->first, additionalTech.begin()->second));
	}
	else
	{
		for(std::vector<std::pair<MetaType, int>>::iterator i = additionalTech.begin(); i != additionalTech.end(); i++)
		{
			newGoal.push_back(std::make_pair((*i).first, (*i).second));
		}
	}
	
	if((currentState.getWorkerCount() >= nextState->getWorkerCount()) &&
		(currentState.getHatcheryCount() >= nextState->getHatcheryCount()) &&
		(currentState.getGasCount() >= nextState->getGasCount()) &&
		additionalTech.empty())
	{
		changeState();
	}

	int sunkensRequired = 1;
	bool sunkensToBuild = false;
	if(threatStatus)
	{
		if((calculateEnemyArmySupply() - armySupply) > 0)
		{
			sunkensRequired += ((calculateEnemyArmySupply() - armySupply) / 20);
		}
		if((sunkensRequired - sunkenCount) > 0)
		{
			sunkensToBuild = true;
		}
	}

	switch(currentState.getTechLevel())
	{
	case 1:
		if(threatStatus)
		{
			if(sunkensToBuild)
			{				
				newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Sunken_Colony, 1));
			}
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Zergling, 4));
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Hydralisk, 3));
		}
		break;
	case 2:
		if(threatStatus)
		{
		//	if(sunkensToBuild)
		//	{
		//		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Sunken_Colony, 1));
		//	}
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Zergling, 2));
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Hydralisk, 4));
		}
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Zergling, 2));
		if(spire)
		{
			newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Mutalisk, 4));
		}
		break;
	case 3:
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Zergling, 12));
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Ultralisk, 3));
		newGoal.push_back(std::make_pair(BWAPI::UnitTypes::Zerg_Mutalisk, 1));
	}

	return newGoal;
}

const void StrategyManager::productionStarted(MetaType element)
{
	if(nextState != NULL)
	{
		if(element.isUpgrade())
		{
			currentState.addRequiredUpgrade(element.upgradeType);
		}
		else if(element.isBuilding())
		{
			currentState.addRequiredBuilding(element.unitType);
		}
	}
}

const void StrategyManager::setEnemyComposition(std::set<std::pair<BWAPI::UnitType, int>> composition)
 {
	 enemyComposition = composition;
 }

const void StrategyManager::addSunken()
{
	sunkenCount++;
}

const void StrategyManager::removeSunken()
{
	sunkenCount--;
}

const void StrategyManager::update(int armyStatus)
{
	int hatcheryCount = 0;
	int workerCount = 0;
	int gasCount = 0;
	int techLevel = 1;
	this->armyStatus = armyStatus;
	spire = false;

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
		if((*i)->getType() == BWAPI::UnitTypes::Zerg_Hive)
		{
			techLevel = 3;
		}
		else if((*i)->getType() == BWAPI::UnitTypes::Zerg_Lair)
		{
			techLevel = 2;
		}
		if(((*i)->getType() == BWAPI::UnitTypes::Zerg_Spire) && ((*i)->isCompleted()))
		{
			spire = true;
		}
	}

	currentState.setTechLevel(techLevel);
	currentState.setWorkerCount(workerCount);
	currentState.setHatcheryCount(hatcheryCount);
	currentState.setGasCount(gasCount);
}

void StrategyManager::updateUpgrades(std::vector<BWAPI::UpgradeType> upgrades)
{
	std::set<BWAPI::UpgradeType> currentUpgrades = currentState.getRequiredUpgrades();

	for(std::vector<BWAPI::UpgradeType>::iterator i = upgrades.begin(); i != upgrades.end(); i++)
	{
		bool found = false;
		for(std::set<BWAPI::UpgradeType>::iterator j = currentUpgrades.begin(); j != currentUpgrades.end(); j++)
		{
			if((*i) == (*j))
			{
				found = true;
			}
		}
		if(!found)
		{
			currentState.addRequiredUpgrade(*i);
		}
	}
}

const void StrategyManager::setThreatStatus(bool threat)
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

const void StrategyManager::setArmyStatus(int status)
{

}

 /*
 prints the current enemy unit composition in terms of percentages of each unit type
 */
const void StrategyManager::drawEnemyInformation(int x, int y)
 {
	int i = 0;

	BWAPI::Broodwar->drawTextScreen(x, y-10,"Enemy Unit Composition:");

	if(enemyComposition.empty())
	{
		BWAPI::Broodwar->drawTextScreen(x, y, "\x04Unknown");
	}

	for(std::set<std::pair<BWAPI::UnitType, int>>::const_iterator unit = enemyComposition.begin(); unit != enemyComposition.end(); unit++)
	{
		BWAPI::Broodwar->drawTextScreen(x, y+(i*10), "\x04%d%% %s", (*unit).second, (*unit).first.getName().c_str());
		i++;
	}
 }

const void StrategyManager::drawStateInformation(int x, int y)
{
	BWAPI::Broodwar->drawTextScreen(x, y, "Current State:");

	if(nextState == &earlyGame)
	{
		BWAPI::Broodwar->drawTextScreen(x, y+10,"\x04 initial state");
	}
	else if(nextState == &midGame)
	{
		BWAPI::Broodwar->drawTextScreen(x, y+10,"\x04 early game");
	}
	else if(nextState == &lateGame)
	{
		BWAPI::Broodwar->drawTextScreen(x, y+10,"\x04 mid game");
	}
	else if(nextState == &finalState)
	{
		BWAPI::Broodwar->drawTextScreen(x, y+10,"\x04 late game");
	}
	else
	{
		BWAPI::Broodwar->drawTextScreen(x, y+10,"\x04 final state");
	}

}

const void StrategyManager::setArmySupply(int supply)
{
	armySupply = supply;
}

int StrategyManager::calculateEnemyArmySupply()
{
	int enemyArmySupply = 0;

	for(std::set<std::pair<BWAPI::UnitType, int>>::iterator i = enemyComposition.begin(); i != enemyComposition.end(); i++)
	{
		enemyArmySupply += ((*i).first.supplyRequired() * (*i).second);
	}

	return enemyArmySupply;
}