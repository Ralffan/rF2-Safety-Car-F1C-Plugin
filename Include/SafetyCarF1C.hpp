//###########################################################################
//#                                                                         #
//# Module: Internals Example Header File                                   #
//#                                                                         #
//# Description: Declarations for the Internals Example Plugin              #
//#                                                                         #
//#                                                                         #
//# This source code module, and all information, data, and algorithms      #
//# associated with it, are part of CUBE technology (tm).                   #
//#                 PROPRIETARY AND CONFIDENTIAL                            #
//# Copyright (c) 2017 Studio 397 B.V.  All rights reserved.               #
//#                                                                         #
//#                                                                         #
//# Change history:                                                         #
//#   tag.2005.11.30: created                                               #
//#                                                                         #
//###########################################################################

#ifndef _INTERNALS_EXAMPLE_H
#define _INTERNALS_EXAMPLE_H

#include "InternalsPlugin.hpp"
#include <string>

// This is used for the app to use the plugin for its intended purpose
class SafetycarF1CPlugin : public InternalsPluginV07  // REMINDER: exported function GetPluginVersion() should return 1 if you are deriving from this InternalsPluginV01, 2 for InternalsPluginV02, etc.
{

 public:

  // Constructor/destructor
  SafetycarF1CPlugin() {}
  ~SafetycarF1CPlugin() {}

  // These are the functions derived from base class InternalsPlugin
  // that can be implemented.
  void Startup( long version );  // game startup
  //void Shutdown();               // game shutdown

  //void EnterRealtime();          // entering realtime
  //void ExitRealtime();           // exiting realtime

  void StartSession();           // session has started
  void EndSession();             // session has ended

  
  // SCORING OUTPUT
  bool WantsScoringUpdates() { return( true ); }
  void UpdateScoring( const ScoringInfoV01 &info );

  bool GetCustomVariable(long i, CustomVariableV01& var);
  void AccessCustomVariable(CustomVariableV01& var);
  //void GetCustomVariableSetting(CustomVariableV01& var, long i, CustomSettingV01& setting);


  // MESSAGE BOX INPUT
  virtual bool WantsToDisplayMessage(MessageInfoV01& msgInfo); // set message and return true

  void DrawSafetyCar();

 private:

  void WriteToOutputFile( const char * const openStr, const char * const msg );
  double mET;  // needed for the hardware example
  bool mEnabled; // needed for the hardware example
  long scProbablity;
  double startLapPercent;
  int startLap;//Primera vuelta en que podría salir el mensaje de SC
  double endLapPercent;
  int endLap;//Última vuelta en que podría salir el mensaje de SC
  int scLap;//Vuelta en la que saldrá el SC (si es que hay), se genera aleatoriamente si el sorteo decide que sí habrá SC
  bool sc;//aquí guardaremos si habrá SC o no
  bool drawdone;
  bool sendmessage;
  bool sccompleted;
  char messageText[128];
  bool sessionStart;
  int randomSC;//número aleatorio de 1 a 100, se genera cuando el líder pasa por la "startLap"
  double scTime;//Guardaremos en qué momento salió el SC
  int reiterateTimes;//Cuántas veces queremos repetir un mensaje
  int messageSent;//Cuántas veces se ha enviado ya el mensaje
  double reiterateInterval;//Cada cuántos segundos reenviamos mensaje
  std::string messageSC[2];

};


#endif // _INTERNALS_EXAMPLE_H

