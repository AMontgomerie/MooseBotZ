/*

Source code by Adam Montgomerie. 
Distributed under GPL v3, see LICENSE for details.

ArmyManager keeps track of all military units under the player's control.
A pointer to each unit is stored in allArmy.
From here, individual units can be accessed so that they can be given orders,
or orders can be issued to all units, or all units of a given UnitType.
*/

#include "ArmyManager.h"
using namespace BWAPI;

ArmyManager::ArmyManager(void) : armyStatus(scout),
								mutaStatus(attack),
								regroupOrdered(false), 
								regroupFrame(0),
								rallyPoint(Position(0,0)),
								attackIssued(false),
								retreatIssued(false)
{
	srand((unsigned int)time(NULL));
}

/*
Should be called each frame.
*/
void ArmyManager::update()
{
	updateStatus();
	drawArmyStatus(320, 10);

	if((armyStatus == attack) && (Broodwar->getFrameCount() % 24 == 0))
	{
		executeAttack();
	}
	if(armyStatus == defend && (Broodwar->getFrameCount() % 24 == 0))
	{
		executeDefence();
	}
	if((armyStatus != defend) && (armyStatus != scout))// && (Broodwar->getFrameCount() % 24 == 0))
	{
		mutaHarass(attackPosition);
	}
	//BWAPI::Broodwar->printf("Us: %d, Them: %d", getArmySupply(), enemyArmySupply);
}

/*
issues commands to army units during an attack
*/
void ArmyManager::executeAttack()
{
	BWAPI::Unit* closestEnemy = NULL;
	BWAPI::Unit* closestUnit = NULL;

	for(std::set<Unit*>::const_iterator i=mainArmy.begin();i!=mainArmy.end();i++)
	{
		//if this is a unit that can't attack but that we want to have with our army (e.g. observer)
		//tell it to follow a friendly army unit that can attack
		if(!(*i)->getType().canAttack() || ((*i)->getType() == BWAPI::UnitTypes::Protoss_Arbiter))
		{
			//find the closest unit to the attack position
			for(std::set<Unit*>::const_iterator j = mainArmy.begin(); j != mainArmy.end(); j++)
			{
				if((closestUnit == NULL) || attackPosition.getDistance((*j)->getPosition()) < attackPosition.getDistance(closestUnit->getPosition()) && (*j)->getType().canAttack())
				{
					closestUnit = (*j);
				}
			}
			if(closestUnit != NULL)
			{
				(*i)->attack(closestUnit->getPosition(), false);
			}
		}
		//find a target for our army units that can attack
		else
		{
			closestEnemy = getClosestEnemy((*i));
			if((closestEnemy != NULL) && closestEnemy->isVisible())
			{
				if((*i)->isUnderAttack())
				{
					(*i)->attack(closestEnemy, false);
				}
				else if((*i)->isIdle())
				{
					(*i)->attack(closestEnemy, false);
				}
			}
			else
			{
				if((*i)->isIdle() && (attackPosition != Position(0,0)) && (attackPosition != NULL))
				{
					(*i)->attack(attackPosition, false);
				}
			}
		}
	}
}

/*
issues commands to our army units during a defence and checks if we still need to defend
once there are no more visible enemy units, return to retreat status
*/
void ArmyManager::executeDefence()
{
	regroupOrdered = false;
	BWAPI::Unit* closestEnemy = NULL;

	if(defendPosition != NULL)
	{
		//if we don't have any fighting units, use workers to defend
		if(allArmy.size() == 0)
		{
			workerCombat();
		}
		//otherwise, use our army to defend
		else
		{
			clearCombatWorkers();

			for(std::set<std::pair<Unit*,int>>::const_iterator i=allArmy.begin();i!=allArmy.end();i++)
			{
				//if this is a unit that can't attack but that we want to have with our army (e.g. observer)
				//tell it to follow a friendly army unit that can attack
				if(!(*i).first->getType().canAttack())
				{
					//find the closest unit to the attack position
					for(std::set<std::pair<Unit*, int>>::const_iterator j = allArmy.begin(); j != allArmy.end(); j++)
					{
						if((closestEnemy == NULL) || attackPosition.getDistance((*j).first->getPosition()) < attackPosition.getDistance(closestEnemy->getPosition()) && (*j).first->getType().canAttack())
						{
							closestEnemy = (*j).first;
						}
					}
					if(closestEnemy != NULL)
					{
						(*i).first->move(closestEnemy->getPosition(), false);
					}
				}
				closestEnemy = getClosestEnemy((*i).first);
				if(closestEnemy != NULL && closestEnemy->isVisible())
				{
					if((*i).first->isUnderAttack())
					{
						(*i).first->attack(closestEnemy, false);
					}
					else if((*i).first->isIdle())
					{
						(*i).first->attack(closestEnemy, false);
					}
				}
				else if((*i).first->isIdle())
				{
					(*i).first->attack(defendPosition, false);
				}
			}
		}
		if((getClosestEnemy(defendPosition) == NULL) || //if there are no enemies nearby
			((getClosestEnemy(defendPosition)->getType().isBuilding()) && !(getClosestEnemy(defendPosition)->getType().canAttack())) || //or the enemy is a building that can't attack
			(getClosestEnemy(defendPosition)->getDistance(defendPosition) > 1000)) //or the closest enemy is too far away
		{
			//clear defense status and return to retreat (will automatically switch to attack if appropriate)
			armyStatus = retreat;
			retreatIssued = true;
			attackIssued = false;
		}
	}
}

/*
finds the closest enemy unit to the given unit and returns a pointer to them
*/
BWAPI::Unit* ArmyManager::getClosestEnemy(BWAPI::Unit* unit)
{
	BWAPI::Unit* closestEnemy = NULL;

//	for(std::set<Unit*>::const_iterator i = Broodwar->enemy()->getUnits().begin(); i != Broodwar->enemy()->getUnits().end(); i++)
	for(std::set<Unit*>::const_iterator i = visibleEnemies.begin(); i != visibleEnemies.end(); i++)
	{
		//find an enemy who...
		if ((closestEnemy == NULL || unit->getDistance(*i) < unit->getDistance(closestEnemy))	//is closer than previous enemies we have checked
			&& (*i)->getType().canAttack()														//can attack (so we prioritise fighting units over workers or buildings
			&& (*i)->isVisible()																//we can see
			&& !(((*i)->isBurrowed() || (*i)->isCloaked()) && !haveDetection()))				//if they are cloaked or burrowed then only target them if we have detection with our army
		closestEnemy = (*i);
	}
	if (closestEnemy == NULL)
	{
		//Broodwar->printf("ArmyManager Error: no enemies found");
	}
	return closestEnemy;
}

/*
finds the closest enemy unit to the given unit and returns a pointer to them
*/
BWAPI::Unit* ArmyManager::getClosestEnemy(BWAPI::Position position)
{
	BWAPI::Unit* closestEnemy = NULL;

	for(std::set<Unit*>::const_iterator i = Broodwar->enemy()->getUnits().begin(); i != Broodwar->enemy()->getUnits().end(); i++)
	{
		if ((closestEnemy == NULL || position.getDistance((*i)->getPosition()) < position.getDistance(closestEnemy->getPosition())) && (*i)->isVisible())
		{
			closestEnemy = (*i);
		}
	}
	if (closestEnemy == NULL)
	{
		//Broodwar->printf("ArmyManager Error: no enemies found");
	}
	return closestEnemy;
}

BWAPI::Unit* ArmyManager::getClosestEnemyMuta(BWAPI::Unit* unit)
{
	BWAPI::Unit* closestEnemy = NULL;
	//closestEnemy = getClosestEnemy(unit);

//	for(std::set<Unit*>::const_iterator i = Broodwar->enemy()->getUnits().begin(); i != Broodwar->enemy()->getUnits().end(); i++)
	for(std::set<Unit*>::const_iterator i = visibleEnemies.begin(); i != visibleEnemies.end(); i++)
	{
		//find an enemy who...
		if ((closestEnemy == NULL || unit->getDistance(*i) < unit->getDistance(closestEnemy))	//is closer than previous enemies we have checked
			&& (*i)->isVisible()	//we can see
			&& (*i)->getType().canAttack()
			&& ((*i)->getType() != BWAPI::UnitTypes::Zerg_Egg)
			&& ((*i)->getType() != BWAPI::UnitTypes::Zerg_Larva)
			&& ((*i)->getType().groundWeapon().targetsAir() || ((*i)->getType().airWeapon() != NULL))
			&& !(((*i)->isBurrowed() || (*i)->isCloaked()) && !haveDetection()))				//if they are cloaked or burrowed then only target them if we have detection with our army
		{
			closestEnemy = (*i);
		}
	}
	if((getClosestEnemyBuilding(unit) != NULL) && ((unit->getDistance(getClosestEnemyBuilding(unit)) < unit->getDistance(closestEnemy)) || (closestEnemy == NULL)))
	{
		if((getClosestEnemyBuilding(unit)->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony) 
			|| (getClosestEnemyBuilding(unit)->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon) 
			|| (getClosestEnemyBuilding(unit)->getType() == BWAPI::UnitTypes::Terran_Missile_Turret)
			|| (getClosestEnemyBuilding(unit)->getType() == BWAPI::UnitTypes::Terran_Bunker))
		{
			closestEnemy = getClosestEnemyBuilding(unit);
		}
	}
	if (closestEnemy == NULL)
	{
		//Broodwar->printf("ArmyManager Error: no enemies found");
	}

	return closestEnemy;
}

BWAPI::Unit* ArmyManager::getClosestEnemyBuilding(BWAPI::Unit* unit)
{
	BWAPI::Unit* closestEnemy = NULL;

//	for(std::set<Unit*>::const_iterator i = Broodwar->enemy()->getUnits().begin(); i != Broodwar->enemy()->getUnits().end(); i++)
	for(std::set<Unit*>::const_iterator i = BWAPI::Broodwar->enemy()->getUnits().begin(); i != BWAPI::Broodwar->enemy()->getUnits().end(); i++)
	{
		//find an enemy who...
		if ((closestEnemy == NULL || unit->getDistance(*i) < unit->getDistance(closestEnemy))	//is closer than previous enemies we have checked														//can attack (so we prioritise fighting units over workers or buildings
			&& (*i)->isVisible()																//we can see
			&& !(((*i)->isBurrowed() || (*i)->isCloaked()) && !haveDetection())
			&& (*i)->getType().isBuilding())				//if they are cloaked or burrowed then only target them if we have detection with our army
		closestEnemy = (*i);
	}
	if (closestEnemy == NULL)
	{
		//Broodwar->printf("ArmyManager Error: no enemies found");
	}
	return closestEnemy;
}

/*
add a unit to the army
*/
void ArmyManager::addUnit(BWAPI::Unit* unit)
{
	allArmy.insert(std::make_pair(unit, 0));

	if(unit->getType() != BWAPI::UnitTypes::Zerg_Mutalisk)
	{
		mainArmy.insert(unit);
	}
	else
	{
		mutas.insert(unit);
	//	mainArmy.insert(unit);
	}
	if(rallyPoint != Position(0,0))
	{
		if(regroupOrdered)
		{
			unit->attack(regroupPosition, false);
		}
		else
		{
			unit->attack(rallyPoint, false);
		}
	}
}

/*
remove a unit from the army
*/
void ArmyManager::removeUnit(BWAPI::Unit* unit)
{
	for(std::set<std::pair<Unit*, int>>::const_iterator i=allArmy.begin();i!=allArmy.end();i++)
	{
		if((*i).first == unit)
		{
			allArmy.erase(*i);
			if(unit->getType() != BWAPI::UnitTypes::Zerg_Mutalisk)
			{
				mainArmy.erase(unit);
			}
			else
			{
				mutas.erase(unit);
				//mainArmy.erase(unit);
			}
			break;
		}
	}
	//check through units that have been issued a regroup command to make sure we aren't going to be waiting for a dead unit to arrive
	for(std::set<Unit*>::const_iterator i = regroupingUnits.begin(); i != regroupingUnits.end(); i++)
	{
		if((*i) == unit)
		{
			regroupingUnits.erase(*i);
			break;
		}
	}
}

/*
returns the number of military units under the player's control
*/
int ArmyManager::getUnitCount()
{
	return allArmy.size();
}

/*
returns the number of units of type unitType under the player's control.
*/
int ArmyManager::getUnitCount(BWAPI::UnitType unitType)
{
	int count = 0;

	if(unitType != NULL)
	{
		for(std::set<std::pair<Unit*, int>>::const_iterator i=allArmy.begin();i!=allArmy.end();i++)
		{
			if((*i).first->getType() == unitType)
			{
				count++;
			}
		}
		return count;
	}
	else
	{
		Broodwar->printf("ArmyManager Error: cannot return count for NULL Unit Type.");
		return 0;
	}
}

/*
tell every unit in the army to attack-move to a position on the map
*/
void ArmyManager::allAttack(BWAPI::Position position)
{
	if(position != NULL)
	{
		for(std::set<std::pair<Unit*, int>>::const_iterator i=allArmy.begin();i!=allArmy.end();i++)
		{
			(*i).first->attack(position, false);
		}
		armyStatus = attack;
		attackPosition = position;
	}
	else
	{
		Broodwar->printf("ArmyManager Error: invalid attack position");
	}
}

void ArmyManager::mainArmyAttack(BWAPI::Position position)
{
	if(position != NULL)
	{
		for(std::set<Unit*>::const_iterator i=mainArmy.begin();i!=mainArmy.end();i++)
		{
			(*i)->attack(position, false);
		}
		armyStatus = attack;
		attackPosition = position;
	}
	else
	{
		Broodwar->printf("ArmyManager Error: invalid attack position");
	}
}

void ArmyManager::mutaAttack(BWAPI::Position position)
{
	if(position != NULL)
	{
		for(std::set<Unit*>::const_iterator i=mutas.begin();i!=mutas.end();i++)
		{
			(*i)->attack(position, false);
		}
		mutaStatus = attack;
		mutaAttackPosition = position;
	}
	else
	{
		Broodwar->printf("ArmyManager Error: invalid attack position");
	}
}


/*
tell every unit in the army to attack a specific unit
*/
void ArmyManager::allAttack(BWAPI::Unit* target)
{
	if(target != NULL)
	{
		for(std::set<std::pair<Unit*, int>>::const_iterator i=allArmy.begin();i!=allArmy.end();i++)
		{
			(*i).first->attack(target, false);
		}
		armyStatus = attack;
	}
	else
	{
		Broodwar->printf("ArmyManager Error: invalid attack target");
	}
}

void ArmyManager::mutaAttack(BWAPI::Unit* target)
{
	if(target != NULL)
	{
		for(std::set<Unit*>::const_iterator i=mutas.begin();i!=mutas.end();i++)
		{
			(*i)->attack(target, false);
		}
		//armyStatus = attack;
	}
	else
	{
		Broodwar->printf("ArmyManager Error: invalid attack target");
	}
}

void ArmyManager::mainArmyAttack(BWAPI::Unit* target)
{
	if(target != NULL)
	{
		for(std::set<Unit*>::const_iterator i=mainArmy.begin();i!=mainArmy.end();i++)
		{
			(*i)->attack(target, false);
		}
		armyStatus = attack;
	}
	else
	{
		Broodwar->printf("ArmyManager Error: invalid attack target");
	}
}

void ArmyManager::allMove(BWAPI::Position position)
{
	if(position != NULL)
	{
		for(std::set<std::pair<Unit*, int>>::const_iterator i=allArmy.begin();i!=allArmy.end();i++)
		{
			(*i).first->move(position, false);
		}
	}
	else
	{
		Broodwar->printf("ArmyManager Error: invalid move position");
	}
}

void ArmyManager::mutaMove(BWAPI::Position position)
{
	if(position != NULL)
	{
		for(std::set<Unit*>::const_iterator i = mutas.begin(); i != mutas.end(); i++)
		{
			(*i)->move(position, false);
		}
	}
	else
	{
		Broodwar->printf("ArmyManager Error: invalid move position");
	}
}

void ArmyManager::mainArmyMove(BWAPI::Position position)
{
	if(position != NULL)
	{
		for(std::set<Unit*>::const_iterator i = mainArmy.begin(); i != mainArmy.end(); i++)
		{
			(*i)->move(position, false);
		}
	}
	else
	{
		Broodwar->printf("ArmyManager Error: invalid move position");
	}
}

/*
returns a single unit of the given UnitType that is under the player's control
*/
BWAPI::Unit* ArmyManager::getUnit(BWAPI::UnitType unitType)
{
	for(std::set<std::pair<Unit*, int>>::const_iterator i=allArmy.begin();i!=allArmy.end();i++)
	{
		if((*i).first->getType() == unitType)
		{
			return (*i).first;
		}
	}
	Broodwar->printf("ArmyManager: you do not control any units of type '%s'", unitType.getName().c_str());
	return NULL;
}

/*
returns a set of all the units of the given UnitType that are under the player's control
*/
std::set<BWAPI::Unit*> ArmyManager::getAllUnitType(BWAPI::UnitType unitType)
{
	std::set<BWAPI::Unit*> units;
	for(std::set<std::pair<Unit*, int>>::const_iterator i=allArmy.begin();i!=allArmy.end();i++)
	{
		if((*i).first->getType() == unitType)
		{
			units.insert((*i).first);
		}
	}
	return units;
}

/*
sets a rally point for new units to gather at
*/
void ArmyManager::setRallyPoint(BWAPI::Position position)
{
	rallyPoint = position;
}

/*
calculates the total supply value of our army
*/
int ArmyManager::getArmySupply()
{
	int armySupply = 0;

	for(std::set<std::pair<BWAPI::Unit*, int>>::const_iterator i = allArmy.begin(); i != allArmy.end(); i++)
	{
		if((*i).first->isCompleted())
		{
			armySupply += (*i).first->getType().supplyRequired();
		}
	}

	return armySupply;
}

/*
returns a sparcraft gamestate including all friendly military units and all visible enemy units
*/
/*
SparCraft::GameState ArmyManager::getState()
{
    SparCraft::GameState state;	
	std::vector<SparCraft::Unit> friendlyUnits;

	//get the current map
	state.setMap(&map);

	int count = 0;
	//add units under our control
	for(std::set<std::pair<Unit*, int>>::iterator i = allArmy.begin(); i != allArmy.end(); i++)
	{
		/*
		//friendlyUnits.push_back(SparCraft::Unit((*i)->getType(), SparCraft::Players::Player_One, (*i)->getPosition()).setUnitID((*i)->getID()));

		SparCraft::Unit u((*i)->getType(), SparCraft::Players::Player_One, (*i)->getPosition());
		u.setUnitID((*i)->getID());
		friendlyUnits.push_back(u);
		//state.addUnitWithID(u);
		*/

/*
		state.addUnit((*i).first->getType(), SparCraft::Players::Player_One, (*i).first->getPosition());
		(*i).second = count;

		count++;
		//Broodwar->printf("adding friendly %s", (*i)->getType().getName().c_str());
	}

	//add enemy units
	for(std::set<Unit*>::const_iterator i = visibleEnemies.begin(); i != visibleEnemies.end(); i++)
	{
		state.addUnit((*i)->getType(), SparCraft::Players::Player_Two, (*i)->getPosition());
		//Broodwar->printf("adding enemy %s", (*i)->getType().getName().c_str());
	}
	
    return state;
}
*/

/*
adds a unit to the set of visible enemies
*/
void ArmyManager::addEnemy(BWAPI::Unit* unit)
{
	visibleEnemies.insert(unit);
}

/*
removes a unit from the set of visible enemies
*/
void ArmyManager::removeEnemy(BWAPI::Unit* unit)
{
	visibleEnemies.erase(unit);
}

void ArmyManager::allRetreat()
{
	if((rallyPoint != NULL) && (rallyPoint != Position(0,0)))
	{
		allMove(rallyPoint);
	}
	else
	{
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isResourceDepot())
			{
				rallyPoint = (*i)->getPosition();
				break;
			}
		}

		allMove(rallyPoint);
	}
	armyStatus = retreat;
}

void ArmyManager::mainArmyRetreat()
{
	if((rallyPoint != NULL) && (rallyPoint != Position(0,0)))
	{
		mainArmyMove(rallyPoint);
	}
	else
	{
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isResourceDepot())
			{
				rallyPoint = (*i)->getPosition();
				break;
			}
		}

		mainArmyMove(rallyPoint);
	}
	armyStatus = retreat;
}

void ArmyManager::mutaRetreat()
{
	if((rallyPoint != NULL) && (rallyPoint != Position(0,0)))
	{
		mutaMove(rallyPoint);
	}
	else
	{
		for(std::set<Unit*>::const_iterator i=Broodwar->self()->getUnits().begin();i!=Broodwar->self()->getUnits().end();i++)
		{
			if ((*i)->getType().isResourceDepot())
			{
				rallyPoint = (*i)->getPosition();
				break;
			}
		}

		mutaMove(rallyPoint);
	}
	mutaStatus = retreat;
}


void ArmyManager::setArmyStatus(int status)
{
	armyStatus = status;
}

void ArmyManager::setDefendPosition(BWAPI::Position position)
{
	defendPosition = position;
}

int ArmyManager::getArmyStatus()
{
	return armyStatus;
}

/*
orders all units to regroup at the location of the unit closest to the attack point
returns true once all units have arrived at that location, false otherwise
*/
bool ArmyManager::regroup()
{
	BWAPI::Unit* closestUnit = NULL;
	bool regroupComplete = false;

	//if we haven't yet ordered the regroup to commence
	if(!regroupOrdered && !mainArmy.empty())
	{
		regroupPosition = rallyPoint;
		mainArmyMove(regroupPosition);

		//feed all current units into the set of units that are regrouping so we can check if they arrive
		for(std::set<Unit*>::const_iterator i = mainArmy.begin(); i != mainArmy.end(); i++)
		{
			if((*i)->getType() != BWAPI::UnitTypes::Zerg_Overlord)
			{
				regroupingUnits.insert(*i);
			}
		}
		regroupOrdered = true;
		regroupFrame = BWAPI::Broodwar->getFrameCount();
	}
	//if the regroup command has already been set
	else
	{
		//check how long it has been since we ordered the regroup command
		if((regroupFrame + MAXREGROUPTIME) < BWAPI::Broodwar->getFrameCount())
		{
			//if it has taken too long then cancel the regroup and continue with the attack
			//this is to prevent us from getting stuck in a situation where we are waiting for a unit that is stuck
			//(for example if we accidently walled it in, or if it is stuck on an island)
			regroupComplete = true;
			regroupOrdered = false;
			return regroupComplete;
		}
		//check that each unit that has been ordered to regroup has arrived at the regroup position
		regroupComplete = true;
		for(std::set<Unit*>::const_iterator i = regroupingUnits.begin(); i != regroupingUnits.end(); i++)
		{
			//check if this unit has arrived
			if(regroupPosition.getDistance((*i)->getPosition()) > REGROUPDIST)
			{
				regroupComplete = false;
			}
			//if the unit is idle and not at the regroup point, reissue the move command
			if((*i)->isIdle() && (regroupPosition.getDistance((*i)->getPosition()) > REGROUPDIST))
			{
				(*i)->move(regroupPosition, false);
			}
		}
	}
	//if the regroup command is successful then reset the variables
	if(regroupComplete)
	{
		regroupOrdered = false;
		regroupingUnits.clear();
	}
	return regroupComplete;
}

void ArmyManager::setAttackPosition(BWAPI::Position position)
{
	if(attackPosition != NULL)
	{
		attackPosition = position;
	}
}

void ArmyManager::drawArmyStatus(int x, int y) 
{
	int i = 0;

	BWAPI::Broodwar->drawTextScreen(x, y-10, "Army Status:");

	if(regroupOrdered)
	{
		BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Regrouping");
	}
	else
	{
		switch(armyStatus)
		{
		case scout:
			BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Scouting");
			break;
		case retreat:
			BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Retreating");
			break;
		case attack:
			BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Attacking");
			break;
		case defend:
			BWAPI::Broodwar->drawTextScreen(x, y, "\x04 Defending");
			break;
		}
	}

}

/*
when we cannot find any enemies, send our units randomly around the map until we find the enemy base
*/
void ArmyManager::findEnemyBase()
{
	BWAPI::Position position;

	armyStatus = scout;

	for(std::set<std::pair<Unit*, int>>::const_iterator i = allArmy.begin(); i != allArmy.end(); i++)
	{
		if((*i).first->isIdle())
		{
			position = Position((rand() % (BWAPI::Broodwar->mapWidth() * 32) + 1), (rand() % (BWAPI::Broodwar->mapHeight() * 32) + 1)).makeValid();

			if((*i).first->hasPath(position))
			{
				(*i).first->attack(position, false);
			}
		}
	}
}

void ArmyManager::updateStatus()
{
	if((armyStatus != defend) && (attackPosition == Position(0,0)))
	{
		findEnemyBase();
		armyStatus = scout;
		attackIssued = false;
		retreatIssued = false;
	}

	if((attackPosition != Position(0,0)) && (attackPosition != NULL))
	{
		if(armyStatus != defend)
		{
			if((enemyArmySupply < getArmySupply()) && !attackIssued)
			{

				if(regroup())
				{
					mainArmyAttack(attackPosition);
					attackIssued = true;
					retreatIssued = false;
				}
			}
			else if(enemyArmySupply >= getArmySupply() && !retreatIssued)
			{
				mainArmyRetreat();
				attackIssued = false;
				retreatIssued = true;
			}
		}
	}
}

void ArmyManager::setEnemyArmySupply(int supply)
{
	enemyArmySupply = supply;
}

/*
get workers and tell them to fight
*/
void ArmyManager::workerCombat()
{
	//calculate how many workers we will need to fight
	size_t threat;

	for(std::set<BWAPI::Unit*>::iterator i = visibleEnemies.begin(); i != visibleEnemies.end(); i++)
	{
		if(!(*i)->getType().isBuilding())
		{
			if((*i)->getType().isWorker())
			{
				threat += (*i)->getType().supplyRequired() / 2;
			}
			else
			{
				threat += (*i)->getType().supplyRequired();
			}
		}
		else
		{
			if((*i)->getType().canAttack())
			{
				threat += 10;
			}
		}
	}

	threat = threat * 2;

	//get the workers
	getCombatWorkers(threat);

	//tell the workers to attack
	if(defendPosition != NULL)
	{
		for(std::set<BWAPI::Unit*>::iterator i = combatWorkers.begin(); i != combatWorkers.end(); i++)
		{
			(*i)->attack(defendPosition);
		}
	}
}

/*
get workers equal to the level of enemy threat
*/
void ArmyManager::getCombatWorkers(size_t threat)
{
	for(std::set<BWAPI::Unit*>::const_iterator i = BWAPI::Broodwar->self()->getUnits().begin(); i != BWAPI::Broodwar->self()->getUnits().end(); i++)
	{
		if(combatWorkers.size() >= threat)
		{
			break;
		}
		if((*i)->getType().isWorker())
		{
			combatWorkers.insert(*i);
		}
	}
}

/*
tell the combat workers to stop fighting so they can return to mining
*/
void ArmyManager::clearCombatWorkers()
{
	for(std::set<BWAPI::Unit*>::iterator i = combatWorkers.begin(); i != combatWorkers.end(); i++)
	{
		(*i)->stop();
	}
	combatWorkers.clear();
}

/*
void ArmyManager::kite()
{
	SparCraft::MoveArray moveArray;
	std::vector<SparCraft::UnitAction> move;
	SparCraft::GameState state;

	SparCraft::PlayerPtr kiter(new SparCraft::Player_Kiter(SparCraft::Players::Player_One));

	if(!allArmy.empty() && !visibleEnemies.empty())// && (BWAPI::Broodwar->getFrameCount() % 24 == 0))
	{
		state = getState();
		state.generateMoves(moveArray, SparCraft::Players::Player_One);
		kiter->getMoves(state, moveArray, move);		
		if(move.empty())
		{
			Broodwar->printf("no moves generated");
		}
		else
		{
			for(std::vector<SparCraft::UnitAction>::iterator unitMove = move.begin(); unitMove != move.end(); unitMove++)
			{
			//	Broodwar->printf("%s", unitMove->moveString().c_str());
				Broodwar->printf("%s", unitMove->debugString().c_str());

				for(std::set<std::pair<Unit*, int>>::const_iterator unit = allArmy.begin(); unit != allArmy.end(); unit++)
				{
					BWAPI::UnitCommand currentCommand((*unit).first->getLastCommand());

					if((*unit).first->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
					{
					//	Broodwar->printf("unit id %d, unitaction id %d", (*unit).first->getID(), (*unitMove).unit());
						if(((*unitMove).unit() == (*unit).second))
						{
						//	Broodwar->printf("%s issued command %d", (*unit).first->getType().getName().c_str(), (*unitMove).type());
							switch((*unitMove).type())
							{
							case 0:	//none
								break;
							case 1: //attack
								if(!(currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Move))
								{
									(*unit).first->attack(BWAPI::Position((*unitMove).pos().x(), (*unitMove).pos().y()), false);
								}
							//	Broodwar->printf("attack!");
								break;
							case 2: //reload
								break;
							case 3: //move
								if(!(currentCommand.getType() == BWAPI::UnitCommandTypes::Move))
								{
									(*unit).first->move(BWAPI::Position((*unitMove).pos().x(), (*unitMove).pos().y()), false);
								}
							//	Broodwar->printf("move!");
								break;
							case 4: //pass
								break;
							case 5: //heal
								break;
							}
							break;
						}
					}
				}
			}
		}
	}
}
*/

bool ArmyManager::haveDetection()
{
	std::set<BWAPI::Unit*> detectors;

	if(BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss)
	{
		detectors = getAllUnitType(BWAPI::UnitTypes::Protoss_Observer);
	}
	else if(BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Zerg)
	{
		return true; //we're always going to have at least 1 overlord (if we don't we're probably dead anyway)
	}

	if(detectors.size() > 0)
	{
		return true;
	}
	return false;
}

void ArmyManager::mutaHarass(BWAPI::Position attackPosition)
{

	for(std::set<Unit*>::const_iterator i=mutas.begin();i!=mutas.end();i++)
	{
		int threatSupply = 0;
		for(std::set<Unit*>::const_iterator e = visibleEnemies.begin(); e != visibleEnemies.end(); e++)
		{
			if(((*e)->getType().airWeapon() != NULL) || (*e)->getType().groundWeapon().targetsAir())
			{
				if((((*e)->getType().airWeapon().maxRange() * 3) >= (*i)->getDistance(*e)) 
					|| ((*e)->getType().groundWeapon().targetsAir() && (((*e)->getType().groundWeapon().maxRange() * 3) >= (*i)->getDistance(*e))))
				{
					threatSupply += (*e)->getType().supplyRequired();
				}
			}
		}
		for(std::set<Unit*>::const_iterator e = BWAPI::Broodwar->enemy()->getUnits().begin(); e != BWAPI::Broodwar->enemy()->getUnits().end(); e++)
		{
			if(((*e)->getType() == BWAPI::UnitTypes::Zerg_Spore_Colony) 
				|| ((*e)->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon) 
				|| ((*e)->getType() == BWAPI::UnitTypes::Terran_Missile_Turret)
				|| ((*e)->getType() == BWAPI::UnitTypes::Terran_Bunker))
			{
				if((*e)->getType() == BWAPI::UnitTypes::Terran_Bunker)
				{
					if((BWAPI::UnitTypes::Terran_Marine.groundWeapon().maxRange() * 3) >= (*i)->getDistance(*e))
					{
						threatSupply += 8;
					}
				}
				else if((((*e)->getType().airWeapon().maxRange() * 3) >= (*i)->getDistance(*e)) 
					|| ((*e)->getType().groundWeapon().targetsAir() && (((*e)->getType().groundWeapon().maxRange() * 3) >= (*i)->getDistance(*e))))
				{
					threatSupply += 8;
				}
			}
		}

		int nearbyMutaSupply = 0;
		for(std::set<Unit*>::const_iterator m = mutas.begin(); m != mutas.end(); m++)
		{
			if(((*i)->getDistance(*m) < 64) && ((*i)->getHitPoints() > 24))
			{
				nearbyMutaSupply += BWAPI::UnitTypes::Zerg_Mutalisk.supplyRequired();
			}
		}

		if((nearbyMutaSupply > (threatSupply * 2)) && !(*i)->isAttacking() && ((*i)->getHitPoints() > 24))
		{
			/*
			BWAPI::Unit* target = getClosestEnemyMuta(*i);
			
			if((target == NULL) || ((getClosestEnemy(*i) != NULL) && (getClosestEnemy(*i)->getDistance(*i) < target->getDistance(*i))))
			{
				target = getClosestEnemy(*i);
			}
			if((target != NULL))
			{
				if(((target->getType().airWeapon().maxRange() * 2) >= (*i)->getDistance(target)) 
					|| (target->getType().groundWeapon().targetsAir() && ((target->getType().groundWeapon().maxRange() * 2) >= (*i)->getDistance(target))))
				{					
				}
				else
				{
					target = getClosestEnemyBuilding(*i);
				}
			}
			if(target != NULL)
			{
				(*i)->attack(target, false);
			}
			else
			{
				(*i)->attack(attackPosition, false);
			}
			*/

			
			if(getClosestEnemyMuta(*i) != NULL)
			{
				(*i)->attack(getClosestEnemyMuta(*i), false);
			}
			else if(getClosestEnemy(*i) != NULL)
			{
				(*i)->attack(getClosestEnemy(*i), false);
			}
			else if(getClosestEnemyBuilding(*i) != NULL)
			{
				(*i)->attack(getClosestEnemyBuilding(*i), false);
			}
			else
			{
				(*i)->attack(attackPosition, false);
			}
			
		}
		else // if(nearbyMutaSupply <= threatSupply)
		{			
			BWAPI::Position position = (*i)->getPosition();
			BWAPI::Position xModifier(64,0);
			BWAPI::Position yModifier(0,64);

			if((getClosestEnemyMuta(*i) != NULL) && !surrounded(*i))
			{
				if((*i)->getPosition().x() > getClosestEnemyMuta(*i)->getPosition().x())
				{
					position += xModifier;
				}
				else if((*i)->getPosition().x() < getClosestEnemyMuta(*i)->getPosition().x())
				{
					position -= xModifier;
				}
				if((*i)->getPosition().y() > getClosestEnemyMuta(*i)->getPosition().y())
				{
					position += yModifier;
				}
				else if((*i)->getPosition().y() < getClosestEnemyMuta(*i)->getPosition().y())
				{
					position -= yModifier;
				}
				(*i)->move(position, false);
			}
			else
			{
				(*i)->move(rallyPoint, false);
			}
			
		}
//		else
//		{
//			(*i)->attack(attackPosition, false);
//		}


		/*
		bool moved = false;
		for(std::set<Unit*>::const_iterator e = visibleEnemies.begin(); e != visibleEnemies.end(); e++)
		{
			if(((*e)->getType().airWeapon() != NULL) || (*e)->getType().groundWeapon().targetsAir())
			{
				if(((*e)->getType().airWeapon().maxRange() >= (*i)->getDistance(*e) + 32) 
					|| ((*e)->getType().groundWeapon().targetsAir() && ((*e)->getType().groundWeapon().maxRange() >= (*i)->getDistance(*e) + 32)))
				{
					moved = true;
					int enemyRange = 0;
					if((*e)->getType().airWeapon() != NULL)
					{
						enemyRange = (*e)->getType().airWeapon().maxRange();
					}
					else
					{
						enemyRange = (*e)->getType().groundWeapon().maxRange();
					}
					(*i)->move(moveOutOfRange((*i)->getPosition(), (*e), enemyRange));
				}
			}
		}
		if(!moved)
		{
			if(getClosestEnemyMuta(*i) == NULL)
			{
				(*i)->attack(attackPosition, false);
			}
			else
			{
				//(*i)->attack(getClosestEnemyMuta(*i), false);
				(*i)->attack(attackPosition, false);
			}
		}
		/*
		int threatSupply = 0;
		int maxThreatRange = 0;

		for(std::set<Unit*>::const_iterator e = visibleEnemies.begin(); e != visibleEnemies.end(); e++)
		{
			if((((*e)->getType().airWeapon() != NULL) || (*e)->getType().groundWeapon().targetsAir()) && ((*i)->getDistance(*e) < 250))
			{
				threatSupply += (*e)->getType().supplyRequired();
				if((*e)->getType().airWeapon().maxRange() > maxThreatRange)
				{
				//	Broodwar->printf("%s has %d range", (*e)->getType().getName().c_str(), (*e)->getType().airWeapon().maxRange());
					maxThreatRange = (*e)->getType().airWeapon().maxRange();
				}
				else if((*e)->getType().groundWeapon().targetsAir() && ((*e)->getType().groundWeapon().maxRange() > maxThreatRange))
				{
				//	Broodwar->printf("%s has %d range", (*e)->getType().getName().c_str(), (*e)->getType().groundWeapon().maxRange());
					maxThreatRange = (*e)->getType().groundWeapon().maxRange();
				}
			}
		}
		//Broodwar->printf("threat range %d", maxThreatRange);

		int nearbyMutaSupply = 0;

		for(std::set<Unit*>::const_iterator m = mutas.begin(); m != mutas.end(); m++)
		{
			if((*i)->getDistance(*m) < 50)
			{
				nearbyMutaSupply += BWAPI::UnitTypes::Zerg_Mutalisk.supplyRequired();
			}
		}

		if(((nearbyMutaSupply) > threatSupply) && !(*i)->isAttacking())// && (Broodwar->getFrameCount() % 24 == 0))
		{
			(*i)->attack(getClosestEnemy(*i), false);
		}
		else if(((*i)->getDistance(getClosestEnemy(*i)) <= maxThreatRange) && !(*i)->isMoving())// && (Broodwar->getFrameCount() % 24 == 0))
		{
			(*i)->move(rallyPoint, false);
		}
		*/
	}
}

bool ArmyManager::surrounded(BWAPI::Unit* muta)
{
	int directionsBlocked = 0;
	bool xPyP = false, xPyM = false, xMyP = false, xMyM = false;

	for(std::set<Unit*>::const_iterator e = visibleEnemies.begin(); e != visibleEnemies.end(); e++)
	{
		bool xPlus = false, xMinus = false, yPlus = false, yMinus = false;

		if(((*e)->getType().airWeapon() != NULL) || (*e)->getType().groundWeapon().targetsAir())
		{
			if((((*e)->getType().airWeapon().maxRange() * 2) >= (muta)->getDistance(*e)) 
				|| ((*e)->getType().groundWeapon().targetsAir() && (((*e)->getType().groundWeapon().maxRange() * 2) >= (muta)->getDistance(*e))))
			{
				if((muta)->getPosition().x() > (*e)->getPosition().x())
				{
					xPlus = true;
				}
				else if((muta)->getPosition().x() < (*e)->getPosition().x())
				{
					xMinus = true;
				}
				if((muta)->getPosition().y() > (*e)->getPosition().y())
				{
					yPlus = true;
				}
				else if((muta)->getPosition().y() < (*e)->getPosition().y())
				{
					yMinus = true;
				}
			}

			if(xPlus && yPlus && !xPyP)
			{
				xPyP = true;
				directionsBlocked++;
			}
			else if(xPlus && yMinus && !xPyM)
			{
				xPyM = true;
				directionsBlocked++;
			}
			else if(xMinus && yPlus && !xMyP)
			{
				xMyP = true;
				directionsBlocked++;
			}
			else if(xMinus && yMinus && !xMyM)
			{
				xMyM = true;
				directionsBlocked++;
			}
			else
			{
			//	BWAPI::Broodwar->printf("bruh");
			}
		}
	}

	//BWAPI::Broodwar->printf("%d directions blocked", directionsBlocked);

	if(directionsBlocked > 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

BWAPI::Position ArmyManager::moveOutOfRange(BWAPI::Position unitPosition, BWAPI::Unit* enemy, int enemyRange)
{
	/*
	BWAPI::Position targetPosition = unitPosition;
	BWAPI::Position shiftPositionX(8,0);
	BWAPI::Position shiftPositionY(0,8);

	if(unitPosition > enemy->getPosition())
	{
	}
	else
	{
	}
	*/

	
	BWAPI::Position targetPosition = unitPosition;
	BWAPI::Position tempPositionXPlus = unitPosition, tempPositionXMinus = unitPosition, tempPositionYPlus = unitPosition, tempPositionYMinus = unitPosition;
	BWAPI::Position shiftPositionX(8,0);
	BWAPI::Position shiftPositionY(0,8);

	if(unitPosition > enemy->getPosition())
	{
		int xCount = 0, yCount = 0;
		while(tempPositionXMinus.getDistance(enemy->getPosition()) < enemyRange)
		{
			tempPositionXMinus -= shiftPositionX;
			xCount++;
		}
		while(tempPositionYMinus.getDistance(enemy->getPosition()) < enemyRange)
		{
			tempPositionYMinus -= shiftPositionY;
			yCount++;
		}
		if(xCount < yCount)
		{
			for(int i = 0; i <= xCount; i++)
			{
				targetPosition -= shiftPositionX;
			}
		}
		else
		{
			for(int i = 0; i <= yCount; i++)
			{
				targetPosition -= shiftPositionY;
			}
		}
	}
	else
	{
		int xCount = 0, yCount = 0;
		while(tempPositionXMinus.getDistance(enemy->getPosition()) < enemyRange)
		{
			tempPositionXMinus += shiftPositionX;
			xCount++;
		}
		while(tempPositionYMinus.getDistance(enemy->getPosition()) < enemyRange)
		{
			tempPositionYMinus += shiftPositionY;
			yCount++;
		}
		if(xCount < yCount)
		{
			for(int i = 0; i <= xCount; i++)
			{
				targetPosition += shiftPositionX;
			}
		}
		else
		{
			for(int i = 0; i <= yCount; i++)
			{
				targetPosition += shiftPositionY;
			}
		}
	}
	/*
	BWAPI::Position targetPosition = unitPosition;
	BWAPI::Position tempPositionXPlus = unitPosition, tempPositionXMinus = unitPosition, tempPositionYPlus = unitPosition, tempPositionYMinus = unitPosition;
	BWAPI::Position shiftPositionX(8,0);
	BWAPI::Position shiftPositionY(0,8);
	int xPlusCount = 0, xMinusCount = 0, yPlusCount = 0, yMinusCount = 0;

	while(tempPositionXPlus.getDistance(enemy->getPosition()) < enemyRange)
	{
		tempPositionXPlus += shiftPositionX;
		xPlusCount++;
	}
	while(tempPositionXMinus.getDistance(enemy->getPosition()) < enemyRange)
	{
		tempPositionXMinus -= shiftPositionX;
		xMinusCount++;
	}
	while(tempPositionYPlus.getDistance(enemy->getPosition()) < enemyRange)
	{
		tempPositionYPlus += shiftPositionY;
		yPlusCount++;
	}
	while(tempPositionYMinus.getDistance(enemy->getPosition()) < enemyRange)
	{
		tempPositionYMinus -= shiftPositionY;
		yMinusCount++;
	}

	if(xPlusCount > xMinusCount)
	{
		for(int i = 0; i <= xPlusCount; i++)
		{
			targetPosition += shiftPositionX;
		}
	}
	else
	{
		for(int i = 0; i <= xMinusCount; i++)
		{
			targetPosition -= shiftPositionX;
		}
	}
	if(yPlusCount > yMinusCount)
	{
		for(int i = 0; i <= yPlusCount; i++)
		{
			targetPosition += shiftPositionY;
		}
	}
	else
	{
		for(int i = 0; i <= yMinusCount; i++)
		{
			targetPosition -= shiftPositionY;
		}
	}
	*/

	return targetPosition;
}