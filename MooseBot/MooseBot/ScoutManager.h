/*
Source code by Adam Montgomerie.
Distributed under GPL v3, see LICENSE for details.
*/

#pragma once
#include <BWAPI.h>
#include <BWTA.h>

class ScoutManager
{
	BWAPI::Unit* scout;
	std::set<BWAPI::Position> knownEnemyBases;
	std::set<BWAPI::Position> knownEnemyMiningBases;
	std::set<std::pair<BWAPI::Unit*, BWAPI::UnitType>> knownEnemyUnits;
	std::set<std::pair<BWAPI::Unit*, BWAPI::UnitType>> enemyStaticD;
	int enemyArmySupply;
	int enemyBaseCount;
	bool enemyCloak;
public:
	ScoutManager();
	~ScoutManager() {}
	static ScoutManager & Instance();
	const void ScoutManager::update();
	const void ScoutManager::setScout(BWAPI::Unit* scout);
	const BWAPI::Unit* ScoutManager::getScout();
	const void ScoutManager::sendScout();
	const void ScoutManager::addEnemyBase(BWAPI::Unit* enemyBase);
	const void ScoutManager::removeEnemyBase(BWAPI::Unit* enemyBase);
	const BWAPI::Position ScoutManager::getClosestEnemyBase(BWAPI::Unit* unit);
	const BWAPI::Position ScoutManager::getEnemyBase();
	const void ScoutManager::addEnemyUnit(BWAPI::Unit* unit);
	const void ScoutManager::removeEnemyUnit(BWAPI::Unit* unit);
	const std::set<std::pair<BWAPI::UnitType, int>> ScoutManager::getEnemyComposition();
	const bool ScoutManager::enemyHasCloak();
	const int ScoutManager::getEnemyArmySupply();
	const void ScoutManager::addEnemyStaticD(BWAPI::Unit* unit);
	const void ScoutManager::removeEnemyStaticD(BWAPI::Unit* unit);
	const int ScoutManager::getTotalEnemyStaticD();
	const void ScoutManager::scoutExpos();
	const int ScoutManager::getEnemyMiningBaseCount();
private:
	void ScoutManager::calculateEnemyArmySupply();
	void ScoutManager::addEnemyMiningBase(BWAPI::Unit* enemyBase);
	void ScoutManager::removeEnemyMiningBase(BWAPI::Unit* enemyBase);
};