/*
Source code by Adam Montgomerie. 
Distributed under GPL v3, see LICENSE for details.
*/

#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <cstdlib>
#include <ctime>

#define REGROUPDIST 500	// the pixel distance a unit must be within to count as having arrived at the regroup location
#define MAXREGROUPTIME 360	//the frame count that we are willing to wait for units to arrive at the regroup location
#define THREATRANGEMODIFIER 3 //the multiplier that is applied to enemy ranges when calculating nearby enemy threats for mutas
#define ALLIEDRADIUS 64 //the radius that is checked for nearby units when calculating the amount of nearby allies

class ArmyManager
{
	std::set<std::pair<BWAPI::Unit*, int>> allArmy;
	std::set<BWAPI::Unit*> mainArmy;
	std::set<BWAPI::Unit*> mutas;
	std::set<BWAPI::Unit*> overlords;
	std::set<std::pair<BWAPI::Position, BWAPI::Unit*>> overlordBaseLocations;
	std::set<BWAPI::Unit*> regroupingUnits;
	std::set<BWAPI::Unit*> visibleEnemies;
	std::set<BWAPI::Unit*> combatWorkers;
	BWAPI::Position attackPosition;
	BWAPI::Position mutaAttackPosition;
	BWAPI::Position defendPosition;
	BWAPI::Position rallyPoint;
	BWAPI::Position regroupPosition;
	int armyStatus;
	int mutaStatus;
	bool regroupOrdered;
	bool attackIssued;
	bool retreatIssued;
	bool analysed;
	int regroupFrame;
	int enemyArmySupply;
public:
	ArmyManager();
	~ArmyManager() {}
	static ArmyManager & Instance();
	const void ArmyManager::update();
	BWAPI::Unit* ArmyManager::getClosestEnemy(BWAPI::Unit* unit);
	BWAPI::Unit* ArmyManager::getClosestEnemy(BWAPI::Position position);
	const void ArmyManager::addUnit(BWAPI::Unit* unit);
	const void ArmyManager::removeUnit(BWAPI::Unit* unit);
	const int ArmyManager::getUnitCount();
	const int ArmyManager::getArmySupply();
	const int ArmyManager::getUnitCount(BWAPI::UnitType unitType);
	const void ArmyManager::allAttack(BWAPI::Position position);
	const void ArmyManager::allAttack(BWAPI::Unit* target);
	const void ArmyManager::allMove(BWAPI::Position position);
	const BWAPI::Unit* ArmyManager::getUnit(BWAPI::UnitType unitType);
	const std::set<BWAPI::Unit*> ArmyManager::getAllUnitType(BWAPI::UnitType unitType);
	const void ArmyManager::setRallyPoint(BWAPI::Position position);
	const void ArmyManager::addEnemy(BWAPI::Unit* unit);
	const void ArmyManager::removeEnemy(BWAPI::Unit* unit);
	const void ArmyManager::allRetreat();
	const void ArmyManager::setDefendPosition(BWAPI::Position position);
	const void ArmyManager::setArmyStatus(int status);
	const void ArmyManager::setArmyStatus();
	const int ArmyManager::getArmyStatus();
	const bool ArmyManager::regroup();
	const void ArmyManager::setAttackPosition(BWAPI::Position position);
	const void ArmyManager::findEnemyBase();
	const void ArmyManager::setEnemyArmySupply(int supply);
	const void ArmyManager::mutaAttack(BWAPI::Position position);
	const void ArmyManager::mutaAttack(BWAPI::Unit* target);
	const void ArmyManager::mutaMove(BWAPI::Position position);
	const void ArmyManager::mainArmyAttack(BWAPI::Position position);
	const void ArmyManager::mainArmyAttack(BWAPI::Unit* target);
	const void ArmyManager::mainArmyMove(BWAPI::Position position);
	const void ArmyManager::mainArmyRetreat();
	const void ArmyManager::mutaRetreat();
	BWAPI::Unit* ArmyManager::getClosestEnemyMuta(BWAPI::Unit* unit);
	BWAPI::Unit* ArmyManager::getClosestEnemyBuilding(BWAPI::Unit* unit);
	const void ArmyManager::analysisFinished();

	enum {scout = 0, retreat = 1, attack = 2, defend = 3};

private:
	void ArmyManager::executeAttack();
	void ArmyManager::executeDefence();
	void ArmyManager::drawArmyStatus(int x, int y);
	void ArmyManager::updateStatus();
	void ArmyManager::workerCombat();
	void ArmyManager::getCombatWorkers(size_t threat);
	void ArmyManager::clearCombatWorkers();
	void ArmyManager::kite();
	bool ArmyManager::haveDetection();
	void mutaHarass(BWAPI::Position attackPosition);
	BWAPI::Position moveOutOfRange(BWAPI::Position unitPosition, BWAPI::Unit* enemy, int enemyRange);
	bool surrounded(BWAPI::Unit* muta);
	void updateOverlords();
	void updateMainArmy();
	BWAPI::Position calculateRegroupPosition();
};
