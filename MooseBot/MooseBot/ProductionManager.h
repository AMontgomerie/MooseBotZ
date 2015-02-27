/*
Source code by Adam Montgomerie.
Distributed under GPL v3, see LICENSE for details.
*/

#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include "BuildingPlacer.h"
#include "WorkerManager.h"
#include "BuildOrderGenerator.h"
#include "BuildOrderQueue.h"


#define MAXWAIT 720*2

class ProductionManager
{
	WorkerManager						workerManager;
	BuildingPlacer						buildingPlacer;
	BuildOrderQueue						production;
	BuildOrderGenerator					buildOrderGenerator;

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
	BWAPI::Unit*						newBase;

	std::vector<std::pair<MetaType, int>>	goal;
	std::vector<BWAPI::UpgradeType>			upgrades;
public:
	ProductionManager();
	~ProductionManager() {}
	static ProductionManager & Instance();
	const void ProductionManager::setBuildOrder(std::vector<MetaType> buildOrder);
	const void ProductionManager::generateBuildOrder(std::vector< std::pair<MetaType, int> > goal);
	const void ProductionManager::update(std::vector< std::pair<MetaType, int> > newGoal);
	const void ProductionManager::checkGas();
	const void ProductionManager::addElement(BuildOrderItem<PRIORITY_TYPE> element);
	const void ProductionManager::removeElement();
	BuildOrderItem<PRIORITY_TYPE> & ProductionManager::getNextElement();
	const bool ProductionManager::emptyQueue();
	const void ProductionManager::addBuilding(BWAPI::Unit* building);
	const void ProductionManager::removeBuilding(BWAPI::Unit* building);
	const void ProductionManager::productionStarted(BWAPI::Unit* unit);
	BWAPI::Unit* ProductionManager::getBuilding(BWAPI::UnitType buildingType);
	BWAPI::Unit* ProductionManager::getBuilding(BWAPI::UnitType buildingType, bool addon);
	const void ProductionManager::removeUnit(BWAPI::Unit* unit);
	const void ProductionManager::addUnit(BWAPI::Unit* unit);
	BWAPI::Unit* ProductionManager::getWorker();
	const void ProductionManager::addGas(BWAPI::Unit* unit);
	const void ProductionManager::produceDetection();
	const void ProductionManager::setCentre(BWAPI::TilePosition centre);
	const void ProductionManager::clearProductionQueue();
	const void ProductionManager::setExpansionStatus(bool status);
	const void ProductionManager::underThreat(bool newThreat);
	const void ProductionManager::updateBuildOrderGenTechLevel(int techLevel);
	const int ProductionManager::getCurrentTechLevel();
	const void ProductionManager::setHomeRegion(BWTA::Region* home);
	std::vector<BWAPI::UpgradeType> ProductionManager::getUpgrades();
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
