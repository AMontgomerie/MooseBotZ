/*
Source code by Adam Montgomerie.
Distributed under GPL v3, see LICENSE for details.

ScoutManager deals with scouting the opponent, managing scouting units and storing information about opponents.
*/

#include "ScoutManager.h"
using namespace BWAPI;

ScoutManager::ScoutManager() :
	scout(NULL),
	enemyCloak(false),
	enemyArmySupply(0),
	enemyBaseCount(0)
{
}

ScoutManager & ScoutManager::Instance()
{
	static ScoutManager instance;

	return instance;
}

/*
Currently redundant - sending worker home now takes place in sendScout
*/
const void ScoutManager::update()
{
	calculateEnemyArmySupply();
}

/*
assigns a unit to be a dedicated scout;
*/
const void ScoutManager::setScout(BWAPI::Unit* scout)
{
	this->scout = scout;
}

/*
returns the scout;
*/
const BWAPI::Unit* ScoutManager::getScout()
{
	return scout;
}

/*
sends the scout to the possible enemy base locations
*/
const void ScoutManager::sendScout()
{
	if(!BWTA::getStartLocations().empty())
	{
		std::set<BWTA::BaseLocation*>::const_iterator i = BWTA::getStartLocations().begin();
		if((*i)->getRegion() != BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion())
		{
			scout->move((*i)->getPosition(), false);
		}
		else
		{
			i++;
			scout->move((*i)->getPosition(), false);
		}
		while(i != BWTA::getStartLocations().end())
		{
			if((*i)->getRegion() != BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion())
			{
				scout->move((*i)->getPosition(), true);
			}
			i++;
		}
		for(std::set<BWAPI::Unit*>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
		{
			if((*i)->getType().isResourceDepot())
			{
				scout->move((*i)->getPosition(), true);
				break; 
			}
		}

	}
	else
	{
		Broodwar->printf("ScoutManager Error: cannot send scout because map has not been analysed");
	}
}



/*
adds an enemy base to the set of known enemy base locations
*/
const void ScoutManager::addEnemyBase(BWAPI::Unit* enemyBase)
{
	knownEnemyBases.insert(enemyBase->getPosition());
	
	if(enemyBase->getType().isResourceDepot())
	{
		addEnemyMiningBase(enemyBase);
	}
}

/*
adds an enemy base to the set of known enemy base locations
*/
void ScoutManager::addEnemyMiningBase(BWAPI::Unit* enemyBase)
{
	knownEnemyMiningBases.insert(enemyBase->getPosition());
}

const void ScoutManager::removeEnemyBase(BWAPI::Unit* enemyBase)
{
	knownEnemyBases.erase(enemyBase->getPosition());

	if(enemyBase->getType().isResourceDepot())
	{
		removeEnemyMiningBase(enemyBase);
	}
}

void ScoutManager::removeEnemyMiningBase(BWAPI::Unit* enemyBase)
{
	knownEnemyMiningBases.erase(enemyBase->getPosition());
}

/*
checks all of the known enemy base locations and returns the one that is closest to the unit* given as a parameter
*/
const BWAPI::Position ScoutManager::getClosestEnemyBase(BWAPI::Unit* unit)
{
	BWAPI::Position closestEnemyBase = Position(0,0);

	if(knownEnemyBases.empty())
	{
		return Position(0,0);
	}

	for(std::set<BWAPI::Position>::iterator i = knownEnemyBases.begin(); i != knownEnemyBases.end() ; i++)
	{
		if ((closestEnemyBase == Position(0,0)) || (unit->getDistance(*i) < unit->getDistance(closestEnemyBase)))
		{
			closestEnemyBase = (*i);
		}
	}

	return closestEnemyBase;
}

/*
returns a known enemy base location
*/
const BWAPI::Position ScoutManager::getEnemyBase()
{	
	if(knownEnemyBases.empty())
	{
		return Position(0,0);
	}

	//refineries are not properly removed from the set of buildings when destroyed
	//if we find a position of a neutral geyser in the set of enemy bases, then remove it
	for(std::set<BWAPI::Unit*>::const_iterator i = BWAPI::Broodwar->getGeysers().begin(); i != BWAPI::Broodwar->getGeysers().end(); i++)
	{
		if(((*i)->getPosition() == *knownEnemyBases.begin()) && (*i)->getType().isNeutral())
		{
			knownEnemyBases.erase((*i)->getPosition());
			break;
		}
	}

	if(knownEnemyBases.empty())
	{
		return Position(0,0);
	}

	return *knownEnemyBases.begin();
}

/*
adds a discovered enemy unit to the set of known enemy units
*/
const void ScoutManager::addEnemyUnit(BWAPI::Unit* unit)
{
	for(std::set<std::pair<BWAPI::Unit*, BWAPI::UnitType>>::const_iterator i = knownEnemyUnits.begin(); i != knownEnemyUnits.end(); i++)
	{
		if((*i).first == unit)
		{
			return;
		}
	}
	knownEnemyUnits.insert(std::make_pair(unit, unit->getType()));
}

/*
removes a destroyed enemy unit from the set of known enemy units
*/
const void ScoutManager::removeEnemyUnit(BWAPI::Unit* unit)
{
	for(std::set<std::pair<BWAPI::Unit*, BWAPI::UnitType>>::const_iterator i = knownEnemyUnits.begin(); i != knownEnemyUnits.end(); i++)
	{
		if(i->first == unit)
		{
			knownEnemyUnits.erase(*i);
			break;
		}
	}
}

/*
Returns a set containing the unit types that make up the enemy unit composition, 
and the percentage that that unit type makes up of the overall composition.
This only includes enemy units that we have scouted so far.
*/
const std::set<std::pair<BWAPI::UnitType, int>> ScoutManager::getEnemyComposition()
{
	std::set<std::pair<BWAPI::UnitType, int>> enemyComposition;
	bool matchFound = false;
	int totalUnits = 0;

	//add up the amounts of each type of enemy unit we know about and the total number of units
	for(std::set<std::pair<BWAPI::Unit*, BWAPI::UnitType>>::const_iterator i = knownEnemyUnits.begin(); i != knownEnemyUnits.end(); i++)
	{
		if((*i).second.isCloakable() || (*i).second.hasPermanentCloak())
		{
			enemyCloak = true;
		}
		matchFound = false;
		for(std::set<std::pair<BWAPI::UnitType, int>>::iterator j = enemyComposition.begin(); j != enemyComposition.end(); j++)
		{
			//if our return set already includes an instance of this unit type
			if((*j).first == (*i).second)
			{
				//add one to the count for this unit type
				(*j).second = (*j).second + 1;
				totalUnits++;
				matchFound = true;
				break;
			}
		}
		//if we haven't recorded an instance of this unit type yet, create a new value in the set
		if(!matchFound)
		{
			enemyComposition.insert(std::make_pair((*i).second, 1));
			totalUnits++;
		}
	}

	//recalculate unit counts for each unit type as a percentage of the overall unit composition
	for(std::set<std::pair<BWAPI::UnitType, int>>::iterator i = enemyComposition.begin(); i != enemyComposition.end(); i++)
	{
		(*i).second = ((*i).second * 100) / totalUnits;
		//Broodwar->printf("%d%% %s", (*i).second, (*i).first.getName().c_str());
	}
	

	return enemyComposition;
}

/*
returns true if we have discovered an enemy unit that can cloak
*/
const bool ScoutManager::enemyHasCloak()
{
	return enemyCloak;
}

/*
updates the total supply of enemy units that we are aware of
*/
void ScoutManager::calculateEnemyArmySupply()
{
	enemyArmySupply = 0;

	for(std::set<std::pair<BWAPI::Unit*, BWAPI::UnitType>>::const_iterator i = knownEnemyUnits.begin(); i != knownEnemyUnits.end(); i++)
	{
		enemyArmySupply += (*i).second.supplyRequired();
	}
}

/*
returns total known enemy army supply
*/
const int ScoutManager::getEnemyArmySupply()
{
	return enemyArmySupply;
}

const void ScoutManager::addEnemyStaticD(BWAPI::Unit* unit)
{
	for(std::set<std::pair<BWAPI::Unit*, BWAPI::UnitType>>::const_iterator i = enemyStaticD.begin(); i != enemyStaticD.end(); i++)
	{
		if((*i).first == unit)
		{
			return;
		}
	}
	enemyStaticD.insert(std::make_pair(unit, unit->getType()));
}

const void ScoutManager::removeEnemyStaticD(BWAPI::Unit* unit)
{
	for(std::set<std::pair<BWAPI::Unit*, BWAPI::UnitType>>::const_iterator i = enemyStaticD.begin(); i != enemyStaticD.end(); i++)
	{
		if(i->first == unit)
		{
			enemyStaticD.erase(*i);
			break;
		}
	}
}

const int ScoutManager::getTotalEnemyStaticD()
{
	int enemyStaticDCount = 0;

	for(std::set<std::pair<BWAPI::Unit*, BWAPI::UnitType>>::const_iterator i = enemyStaticD.begin(); i != enemyStaticD.end(); i++)
	{
		enemyStaticDCount++;
	}
	return enemyStaticDCount;
}

const void ScoutManager::scoutExpos()
{
	if(!BWTA::getBaseLocations().empty())
	{
		scout->move((*BWTA::getBaseLocations().begin())->getPosition(), false);
		for(std::set<BWTA::BaseLocation*>::const_iterator i = BWTA::getBaseLocations().begin(); i != BWTA::getBaseLocations().end(); i++)
		{
			scout->move((*i)->getPosition(), true);
		}
	}
	else
	{
		return;
	}
	for(std::set<BWAPI::Unit*>::const_iterator i = Broodwar->self()->getUnits().begin(); i != Broodwar->self()->getUnits().end(); i++)
	{
		if((*i)->getType().isResourceDepot())
		{
			scout->move((*i)->getPosition(), true);
			break; 
		}
	}
}

const int ScoutManager::getEnemyMiningBaseCount()
{
	return knownEnemyMiningBases.size();
}