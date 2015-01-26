#pragma once
#include "Metatype.h"

class BuildOrderGenerator
{
	int techLevel;
public:
	BuildOrderGenerator(void);
	std::vector<MetaType> generateBuildOrder(std::vector<std::pair<MetaType, int>> goal, std::set<BWAPI::Unit*> buildings);
	std::vector<MetaType> getOpeningBuildOrder();
	int getTechLevel();
	void setTechLevel(int techLevel);
private:
	std::vector<MetaType> queueUnits(std::vector<MetaType> buildOrder, std::vector< std::pair<MetaType, int> > goal);
	int calculateSupplyRequired(std::vector<std::pair<MetaType, int>> goal);
	bool checkBuilding(std::vector<std::pair<MetaType, int>> goal, std::set<BWAPI::Unit*> buildings);
	bool compareBuildings(BWAPI::UnitType required, std::set<BWAPI::Unit*> buildings);
	std::vector<MetaType> createMoreSupply(std::vector<MetaType> buildOrder, int totalSupplyRequired, int remainingSupply);
	std::vector<MetaType> increaseTechLevel(std::vector<MetaType> buildOrder);
	bool checkTech(BWAPI::UnitType requiredUnit);
};

