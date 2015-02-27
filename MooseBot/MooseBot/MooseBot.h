/*
Source code by Adam Montgomerie. Modified from ExampleAIModule from BWAPI 3.7.4
Distributed under GPL v3, see LICENSE for details.
*/

#pragma once

#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include "Timer.hpp"
#include "ProductionManager.h"
#include "ArmyManager.h"
#include "ScoutManager.h"
#include "StrategyManager.h"

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

class MooseBot: public BWAPI::AIModule
{
	bool underThreat;
public:
  virtual void onStart();
  //virtual void onEnd(bool isWinner);
  virtual void onFrame();
  //virtual void onSendText(std::string text);
  //virtual void onReceiveText(BWAPI::Player* player, std::string text);
  //virtual void onPlayerLeft(BWAPI::Player* player);
  //virtual void onNukeDetect(BWAPI::Position target);
  //virtual void onUnitDiscover(BWAPI::Unit* unit);
  //virtual void onUnitEvade(BWAPI::Unit* unit);
  virtual void onUnitShow(BWAPI::Unit* unit);
  virtual void onUnitHide(BWAPI::Unit* unit);
  virtual void onUnitCreate(BWAPI::Unit* unit);
  virtual void onUnitDestroy(BWAPI::Unit* unit);
  virtual void onUnitMorph(BWAPI::Unit* unit);
  //virtual void onUnitRenegade(BWAPI::Unit* unit);
  //virtual void onSaveGame(std::string gameName);
  virtual void onUnitComplete(BWAPI::Unit *unit);
  void drawStats(); //not part of BWAPI::AIModule
  void drawBullets();
  void drawVisibilityData();
  void drawTerrainData();
  void showPlayers();
  void showForces();
  bool show_bullets;
  bool show_visibility_data;
  void MooseBot::checkUnderAttack();
  void MooseBot::drawTimerInformation(int x, int y, double t);
  enum {scout = 0, retreat = 1, attack = 2, defend = 3};
};