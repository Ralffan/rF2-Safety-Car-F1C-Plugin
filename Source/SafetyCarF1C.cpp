//###########################################################################
//#                                                                         #
//# Module: Internals Example Source File                                   #
//#                                                                         #
//# Description: Declarations for the Internals Example Plugin              #
//#                                                                         #
//#                                                                         #
//# This source code module, and all information, data, and algorithms      #
//# associated with it, are part of CUBE technology (tm).                   #
//#                 PROPRIETARY AND CONFIDENTIAL                            #
//# Copyright (c) 2018 Studio 397 B.V.  All rights reserved.                #
//#                                                                         #
//###########################################################################

#include "SafetyCarF1C.hpp"          // corresponding header file
#include <math.h>               // for atan2, sqrt
#include <stdio.h>              // for sample output
#include <string>
#include <random>


// plugin information

extern "C" __declspec( dllexport )
const char * __cdecl GetPluginName()                   { return( "SafetycarF1CPlugin" ); }

extern "C" __declspec( dllexport )
PluginObjectType __cdecl GetPluginType()               { return( PO_INTERNALS ); }

extern "C" __declspec( dllexport )
int __cdecl GetPluginVersion()                         { return( 7 ); } // InternalsPluginV01 functionality (if you change this return value, you must derive from the appropriate class!)

extern "C" __declspec( dllexport )
PluginObject * __cdecl CreatePluginObject()            { return( (PluginObject *) new SafetycarF1CPlugin); }

extern "C" __declspec( dllexport )
void __cdecl DestroyPluginObject( PluginObject *obj )  { delete( (SafetycarF1CPlugin *) obj ); }


// ExampleInternalsPlugin class

void SafetycarF1CPlugin::WriteToOutputFile( const char * const openStr, const char * const msg )
{
  FILE *fo;

  fo = fopen( "SafetyCarF1Clog.txt", openStr );
  if( fo != NULL )
  {
    fprintf( fo, "%s\n", msg );
    fclose( fo );
  }

}



void SafetycarF1CPlugin::Startup( long version )
{
    //inicializamos variables al cargar el juego
    sc = false;
    drawdone = false;
    sendmessage = false;
    sccompleted = false;
    sessionStart = false;
    randomSC = 0;
    //reiterateTimes = 2;//veces que se volverá a repetir el mensaje de SC
    //reiterateInterval = 11.0;
    messageSent = 0;
    WriteToOutputFile("a", "startup");
}


void SafetycarF1CPlugin::StartSession()
{
    //nos interesa reiniciar algunas variabless si se reinicia la sesión
    sc = false;
    drawdone = false;
    sendmessage = false;
    sccompleted = false;
    sessionStart = true;
    sccompleted = false;
    randomSC = 0;
    messageSent = 0;
    messageSC[0] = "**SAFETY CAR next lap**";
    messageSC[1] = "**SAFETY CAR vuelta siguiente**";
    //WriteToOutputFile( "a", "--STARTSESSION--");
}


void SafetycarF1CPlugin::EndSession()
{
    //Al acabar la carrera, guardamos en un log el resultados de los sorteos, por si queremos consultar o analizar si hay más o menos SCs de lo esperado
    if (drawdone) {
        std::string randomSC_str = "Resultado sorteo: " + std::to_string(randomSC);
        WriteToOutputFile("a", randomSC_str.c_str());
        std::string sclap_str = "Resultado sorteo vuelta: " + std::to_string(scLap);
        WriteToOutputFile("a", sclap_str.c_str());
        WriteToOutputFile("a", "--ENDSESSION--");
    }
}

void SafetycarF1CPlugin::DrawSafetyCar()//Esta función es la que decide aleatoriamente si hay o no SC y en qué vuelta. Solo se llamará 1 vez
{
    //más info sobre librería "random" para generar números aleatorios: https://learn.microsoft.com/es-es/cpp/standard-library/random?view=msvc-170
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    randomSC = dist(gen);//Generamos un número entero aleatorio del 1 al 100 (incluídos)

    //Si el número es más pequeño o igual que la probabilidad de SC, hay qué sortear en qué vuelta sale
    if (randomSC <= scProbablity) {
        sc = true;//Guardamos que sí hay SC en este caso
        std::random_device rd2;
        std::mt19937 gen2(rd2());
        std::uniform_int_distribution<> dist2(startLap, endLap);//se genera distribución uniforme de números enteros de todas las vueltas en que puede haber SC

        scLap = dist2(gen);//Generamos un número entero aleatorio comprendido en el intervalo anterior
    }
}



void SafetycarF1CPlugin::UpdateScoring( const ScoringInfoV01 &info )
{  // Note: function is called twice per second now (instead of once per second in previous versions)

    // Nothing outside of the race session for now
    if (info.mSession < 10) return; // 10-13 are race session


    //Una vez se ha hecho el sorteo, si no hay SC no nos interesa hacer nada más
    //Si todo el proceso de SC ya se ha completado (se ha enviado el mensaje de Safety Car), tampoco nos interesa hacer nada más
    if ((drawdone && !sc) || (sccompleted && (messageSent > reiterateTimes))) return;

        

    if (sccompleted && (messageSent <= reiterateTimes)) {//Repetimos el mensaje cada reiterateInterval las veces definidas
        if (info.mCurrentET >= ((messageSent * reiterateInterval) + scTime)) {
            strcpy(messageText, messageSC[messageSent % 2].c_str());
            sendmessage = true;
            messageSent++;
        }
        return;
    }

    if (sessionStart) {
        //Al inicio de carrera, envíamos mensaje al hat recordando la probabilidad de SC para este GP y entre qué vueltas puede salir
        //En el CustomVariablesPlugins se guardan las vueltas a modo de porcentaje, así que debemos calcular a qué vueltas equivalen según la distancia de carrera
        //Ejemplo: la primera vuelta redondeando hacia arriba siempre, la última, redondeando hacia abajo
        startLap = ceil((info.mMaxLaps * (startLapPercent / 100.0)) - 1e-9);//Añadimos pequeño margen de error porque hemos pasado de entero a double
        endLap = floor((info.mMaxLaps * (endLapPercent / 100.0)) + 1e-9);
        std::string message = "Probabilidad de SC: " + std::to_string(scProbablity) + "% " + "entre las vueltas " + std::to_string(startLap) + " y " + std::to_string(endLap);
        strcpy(messageText, message.c_str());
        sendmessage = true;
        sessionStart = false;
        WriteToOutputFile("a", message.c_str());
        return;
    }



    for( long i = 0; i < info.mNumVehicles; ++i )
    {
      VehicleScoringInfoV01 &vinfo = info.mVehicle[ i ];
      if (!drawdone && vinfo.mTotalLaps >= startLap - 1) {//La primera vez que alguien cruza por meta la primera posible vuelta de SC, se hace el sorteo
          WriteToOutputFile("a", info.mTrackName);
          std::string mensaje = "SORTEO SC en " + std::to_string(info.mCurrentET) + " generado por piloto " + vinfo.mDriverName;
          WriteToOutputFile("a", mensaje.c_str());
          mensaje = "Probabilidad de SC: " + std::to_string(scProbablity);
          WriteToOutputFile("a", mensaje.c_str());
          SafetycarF1CPlugin::DrawSafetyCar();
          drawdone = true;
          return;
      }
      else if (drawdone && sc && !sendmessage) {//Si llegamos aquí, significa que en el sorteo ha salido que hay SC, pero aun no ha salido
          if (vinfo.mTotalLaps + 1 == scLap && vinfo.mSector == 2 ) {//Ponemos el mensaje cuando el lider cruza el primer sector de la vuelta en que debe salir el mensaje
              strcpy(messageText, messageSC[0].c_str());
              sendmessage = true;
              messageSent++;
              scTime = info.mCurrentET;
              sccompleted = true;
              //Guardamos en log el momento en que sale el mensaje de SC
              std::string mensaje2 = "SC deployment triggered at " + std::to_string(scTime) + " by " + vinfo.mDriverName;
              WriteToOutputFile("a", mensaje2.c_str());
              return;
          }
      }
    }
}


bool SafetycarF1CPlugin::WantsToDisplayMessage(MessageInfoV01 &msgInfo) {
    if (sendmessage)
    {
        strcpy(msgInfo.mText, messageText);               // message to display

        msgInfo.mDestination = 1;    // 0 = message center, 1 = chat (can be used for multiplayer chat commands)
        msgInfo.mTranslate = 0;      // 0 = do not attempt to translate, 1 = attempt to translate

        //unsigned char mExpansion[126]; // for future use (possibly what color, what font, and seconds to display)
        sendmessage = false;
        
        return true;
    }
    return false;
}


bool SafetycarF1CPlugin::GetCustomVariable(long i, CustomVariableV01& var)
{
    switch (i)
    {
        case 0:
        {
            // rF2 will automatically create this variable and default it to 1 (true) unless we create it first, in which case we can choose the default.
            strcpy_s(var.mCaption, " Enabled");
            var.mNumSettings = 2;
            var.mCurrentSetting = 0;
            return(true);
        }
        // return before break;

        case 1:
        {
            strcpy_s(var.mCaption, "SCprobability");
            var.mNumSettings = 100;
            var.mCurrentSetting = 10;
            return(true);
        }
        // return before break;

        case 2:
        {
            strcpy_s(var.mCaption, "minLapPercent");
            var.mNumSettings = 101;
            var.mCurrentSetting = 20;
            return(true);
        }
        // return before break;

        case 3:
        {
            strcpy_s(var.mCaption, "maxLapPercent");
            var.mNumSettings = 101;
            var.mCurrentSetting = 70;
            return(true);
        }

        case 4:
        {
            strcpy_s(var.mCaption, "reiterateTimes");
            var.mNumSettings = 20;
            var.mCurrentSetting = 2;
            return(true);
        }
        case 5:
        {
            strcpy_s(var.mCaption, "reiterateInterval");
            var.mNumSettings = 30;
            var.mCurrentSetting = 11;
            return(true);
        }

    // return before break;
    }

    return(false);
}

void SafetycarF1CPlugin::AccessCustomVariable(CustomVariableV01& var)
{
    if (0 == _stricmp(var.mCaption, " Enabled"))
    {
        // Do nothing; this variable is just for rF2 to know whether to keep the plugin loaded.
    }
    else if (0 == _stricmp(var.mCaption, "SCprobability"))
    {
        scProbablity = var.mCurrentSetting;
    }
    else if (0 == _stricmp(var.mCaption, "minLapPercent"))
    {
        startLapPercent = var.mCurrentSetting;
    }
    else if (0 == _stricmp(var.mCaption, "maxLapPercent"))
    {
        endLapPercent = var.mCurrentSetting;
    }
    else if (0 == _stricmp(var.mCaption, "reiterateTimes"))
    {
        reiterateTimes = var.mCurrentSetting;
    }
    else if (0 == _stricmp(var.mCaption, "reiterateInterval"))
    {
        reiterateInterval = var.mCurrentSetting;
    }
    else
    {
    }
}