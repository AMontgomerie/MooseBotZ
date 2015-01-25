#include "BuildOrderGenerator.h"

BuildOrderGenerator::BuildOrderGenerator(void)
{
	techLevel = 1;
}

//takes a set of goal units and their respective unit counts and returns a build order to produce them
std::vector<MetaType> BuildOrderGenerator::generateBuildOrder(std::vector< std::pair<MetaType, int> > goal, std::set<BWAPI::Unit*> buildings)
{
	std::vector<MetaType> buildOrder;

	bool haveRequiredBuilding = checkBuilding(goal, buildings);

	//check if we have all the tech we need to build the units
	if(haveRequiredBuilding)
	{
		//calculate the total supply required by the goal units
		int totalSupplyRequired = calculateSupplyRequired(goal);

		int remainingSupply = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();

		//if we already have enough supply to make the units then go ahead and queue them up
		if(remainingSupply > totalSupplyRequired)
		{
			buildOrder = queueUnits(buildOrder, goal);
		}
		//otherwise we need to work out how much supply we need and build that many overlords
		else
		{
			buildOrder = createMoreSupply(buildOrder, totalSupplyRequired, remainingSupply);
			//once we've queued up the overlords then we can queue up the units we want
			buildOrder = queueUnits(buildOrder, goal);
		}
	}
	//if we don't have the required tech then queue it up for production
	else
	{
	//	BWAPI::Broodwar->printf("we don't have the right tech");
		for(std::vector< std::pair<MetaType, int> >::const_iterator g = goal.begin(); g != goal.end(); g++)
		{
			for(std::map<BWAPI::UnitType, int>::const_iterator r = g->first.unitType.requiredUnits().begin(); r != g->first.unitType.requiredUnits().end(); r++)
			{
				if(!compareBuildings(r->first, buildings))
				{
					if(!checkTech(r->first))
					{
						buildOrder = increaseTechLevel(buildOrder);
					}
					buildOrder.push_back(MetaType(r->first));
				}
			}
		}
		//once we have the correct tech queued up then we can move on to checking supply

		//calculate the total supply required by the goal units
		int totalSupplyRequired = calculateSupplyRequired(goal);

		int remainingSupply = BWAPI::Broodwar->self()->supplyTotal() - BWAPI::Broodwar->self()->supplyUsed();

		//if we already have enough supply to make the units then go ahead and queue them up
		if(remainingSupply > totalSupplyRequired)
		{
			buildOrder = queueUnits(buildOrder, goal);
		}
		//otherwise we need to work out how much supply we need and build that many overlords
		else
		{
			buildOrder = createMoreSupply(buildOrder, totalSupplyRequired, remainingSupply);
			//once we've queued up the overlords then we can queue up the units we want
			buildOrder = queueUnits(buildOrder, goal);
		}
	}

	return buildOrder;
}

//upgrade to lair or hive depending on our current tech level
std::vector<MetaType> BuildOrderGenerator::increaseTechLevel(std::vector<MetaType> buildOrder)
{
	switch(techLevel)
	{
	case 1:
		buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Lair));
		techLevel = 2;
		break;
	case 2:
		buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hive));
		techLevel = 3;
		break;
	case 3:
	default:
		BWAPI::Broodwar->printf("u wot m8");
		break;
	}

	return buildOrder;
}

std::vector<MetaType> BuildOrderGenerator::queueUnits(std::vector<MetaType> buildOrder, std::vector< std::pair<MetaType, int> > goal)
{	
	for(std::vector< std::pair<MetaType, int> >::const_iterator g = goal.begin(); g != goal.end(); g++)
	{
		int count = g->second;
		while(count > 0)
		{
		//	BWAPI::Broodwar->printf("adding %s", g->first.unitType.getName().c_str());
			buildOrder.push_back(MetaType(g->first.unitType));
			count--;
		}
	}

	return buildOrder;
}

std::vector<MetaType> BuildOrderGenerator::createMoreSupply(std::vector<MetaType> buildOrder, int totalSupplyRequired, int remainingSupply)
{
	int overlordsRequired = (((totalSupplyRequired/2) - remainingSupply) / 8) + 1;
//	BWAPI::Broodwar->printf("supply required is %d, remaining supply is %d, overlords required is %d", totalSupplyRequired, remainingSupply, overlordsRequired);

	int count = overlordsRequired;
	while(count > 0)
	{
		buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Overlord));
		count--;
	}

	return buildOrder;
}

//returns a scripted opening build order
std::vector<MetaType> BuildOrderGenerator::getOpeningBuildOrder()
{
	std::vector<MetaType> buildOrder;

	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Overlord));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Spawning_Pool));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Extractor));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Hatchery));
//	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Drone));
//	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Zergling));
//	buildOrder.push_back(MetaType(BWAPI::UnitTypes::Zerg_Zergling));


	return buildOrder;
}

//calculate how much supply a set of goal units requires
int BuildOrderGenerator::calculateSupplyRequired(std::vector<std::pair<MetaType, int>> goal)
{
	int totalSupplyRequired = 0;

	for(std::vector< std::pair<MetaType, int> >::const_iterator g = goal.begin(); g != goal.end(); g++)
	{
		totalSupplyRequired += (g->first.unitType.supplyRequired() * g->second);
	}

	return totalSupplyRequired;
}

//check that we have the tech to produce each of the required units
bool BuildOrderGenerator::checkBuilding(std::vector<std::pair<MetaType, int>> goal, std::set<BWAPI::Unit*> buildings)
{
	for(std::vector< std::pair<MetaType, int> >::const_iterator g = goal.begin(); g != goal.end(); g++)
	{
		for(std::map<BWAPI::UnitType, int>::const_iterator r = g->first.unitType.requiredUnits().begin(); r != g->first.unitType.requiredUnits().end(); r++)
		{
			if(!compareBuildings(r->first, buildings))
			{
				return false;
			}
		}
	}
	return true;
}

//check if the required unit type is in the set of buildings that we currently have
bool BuildOrderGenerator::compareBuildings(BWAPI::UnitType required, std::set<BWAPI::Unit*> buildings)
{
	//BWAPI::Broodwar->printf("%s required", required.getName().c_str());
	//ignore if we are checking for larvae because they are produced automatically and aren't including in the building set
	//drones also aren't in the building set so ignore them too
	if((required == BWAPI::UnitTypes::Zerg_Larva) || (required == BWAPI::UnitTypes::Zerg_Drone))
	{
		return true;
	}

	for(std::set<BWAPI::Unit*>::const_iterator b = buildings.begin(); b != buildings.end(); b++)
	{
		if((*b)->getType() == required)
		{
			return true;
		}
	}
	//BWAPI::Broodwar->printf("missing tech is %s", required.getName().c_str());
	return false;
}

bool BuildOrderGenerator::checkTech(BWAPI::UnitType requiredUnit)
{
	if(((requiredUnit == BWAPI::UnitTypes::Zerg_Lurker) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Mutalisk) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Nydus_Canal) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Queen) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Queens_Nest) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Scourge) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Spire)) && (techLevel < 2))
	{
		return false;
	}
	else if(((requiredUnit == BWAPI::UnitTypes::Zerg_Defiler) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Defiler_Mound) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Greater_Spire) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Devourer) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Guardian) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Ultralisk) ||
		(requiredUnit == BWAPI::UnitTypes::Zerg_Ultralisk_Cavern)) && (techLevel < 3))
	{
		return false;
	}
	return true;
}

int BuildOrderGenerator::getTechLevel()
{
	return techLevel;
}