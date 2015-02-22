/*
Source code by Adam Montgomerie.
Distributed under GPL v3, see LICENSE for details.
*/

#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include "BuildOrderQueue.h"
#include "BuildingPlacer.h"
#include "WorkerManager.h"
#include "BuildOrderGenerator.h"
#include "StrategyManager.h"


#define MAXWAIT 720*2

class ProductionManager
{
	WorkerManager						workerManager;
	BuildingPlacer						buildingPlacer;
	BuildOrderQueue						production;
	BuildOrderGenerator					buildOrderGenerator;
	StrategyManager						strategyManager;

	std::set<BWAPI::Unit*>				buildings;
	std::vector<BWAPI::Unit*>			gas;
	BWAPI::TilePosition					nextExpansionLocation;
	bool								expanding;
	bool								deadlockfound;
	bool								expansionQueued;
	bool								expandingIsAdvisable;
	bool								currentThreat;
	BWAPI::TilePosition					centre;
	int									lastProductionFrame;
	int									lastExpansionFrame;
	int techLevel;
	BWAPI::Unit*						homeBase;
	BWTA::Region*						homeRegion;

	std::vector< std::pair<MetaType, int> > goal;
public:
	ProductionManager::ProductionManager();
	void ProductionManager::setBuildOrder(std::vector<MetaType> buildOrder);
	void ProductionManager::generateBuildOrder(std::vector< std::pair<MetaType, int> > goal);
	void ProductionManager::update(int armyStatus);
	void ProductionManager::checkGas();
	void ProductionManager::addElement(BuildOrderItem<PRIORITY_TYPE> element);
	void ProductionManager::removeElement();
	BuildOrderItem<PRIORITY_TYPE> & ProductionManager::getNextElement();
	bool ProductionManager::emptyQueue();
	void ProductionManager::addBuilding(BWAPI::Unit* building);
	void ProductionManager::removeBuilding(BWAPI::Unit* building);
	void ProductionManager::productionStarted(BWAPI::Unit* unit);
	BWAPI::Unit* ProductionManager::getBuilding(BWAPI::UnitType buildingType);
	BWAPI::Unit* ProductionManager::getBuilding(BWAPI::UnitType buildingType, bool addon);
	void ProductionManager::removeUnit(BWAPI::Unit* unit);
	void ProductionManager::addUnit(BWAPI::Unit* unit);
	BWAPI::Unit* ProductionManager::getWorker();
	void ProductionManager::addGas(BWAPI::Unit* unit);
	void ProductionManager::produceDetection();
	void ProductionManager::setCentre(BWAPI::TilePosition centre);
	void ProductionManager::clearProductionQueue();
	void ProductionManager::setExpansionStatus(bool status);
	void ProductionManager::underThreat(bool newThreat);
	void ProductionManager::updateBuildOrderGenTechLevel(int techLevel);
	int ProductionManager::getCurrentTechLevel();
	void ProductionManager::setHomeRegion(BWTA::Region* home);
	void ProductionManager::setEnemyComposition(std::set<std::pair<BWAPI::UnitType, int>> composition);
	void ProductionManager::setArmySupply(int supply);
	void ProductionManager::addSunken();
	void ProductionManager::removeSunken();
private:
	void ProductionManager::createUnit(BuildOrderItem<PRIORITY_TYPE> element);
	void ProductionManager::createAddon(BuildOrderItem<PRIORITY_TYPE> element);
	void ProductionManager::morphUnit(BuildOrderItem<PRIORITY_TYPE> element);
	void ProductionManager::createBuilding(BuildOrderItem<PRIORITY_TYPE> element);
	void ProductionManager::startUpgrade(BuildOrderItem<PRIORITY_TYPE> element);
	bool ProductionManager::canAfford(BuildOrderItem<PRIORITY_TYPE> element);
	void ProductionManager::beginProduction(BuildOrderItem<PRIORITY_TYPE> element);
	void ProductionManager::drawGoalInformation(int x, int y, std::vector< std::pair<MetaType, int> > goal);
	void ProductionManager::checkForDeadlock();
	void ProductionManager::checkMinerals();
	void ProductionManager::removeUnwantedItems();
	bool ProductionManager::isTechBuilding(BuildOrderItem<PRIORITY_TYPE> element);
	BWAPI::Unit* ProductionManager::getUncompletedBuilding(BWAPI::UnitType buildingType);
	std::set<BWAPI::Unit*> ProductionManager::getAllLarvae();
	BWAPI::TilePosition ProductionManager::determineBuildPosition(BWAPI::UnitType structureType);
};
