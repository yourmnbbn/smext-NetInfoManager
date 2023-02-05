/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "extension.h"
#include <CDetour/detours.h>

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

NetInfoMgr g_NetInfoMgr;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_NetInfoMgr);

CDetour*        g_pHostRunFrame;
IGameConfig*    g_pGameConfig = nullptr;
IForward*       g_pOnFrameDataUpdatedForward = nullptr;

float* host_frameendtime_computationduration = nullptr;
float* host_frametime_stddeviation = nullptr;
float* host_framestarttime_stddeviation = nullptr;

void OnFrameDataUpdated()
{
    const float flRatio = 1000.0;

    float frameendtime_computationduration = *host_frameendtime_computationduration*flRatio;
    float frametime_stddeviation = *host_frametime_stddeviation*flRatio;
    float framestarttime_stddeviation = *host_framestarttime_stddeviation*flRatio;

    cell_t result;
    g_pOnFrameDataUpdatedForward->PushFloatByRef(&frameendtime_computationduration);
    g_pOnFrameDataUpdatedForward->PushFloatByRef(&frametime_stddeviation);
    g_pOnFrameDataUpdatedForward->PushFloatByRef(&framestarttime_stddeviation);
    g_pOnFrameDataUpdatedForward->Execute(&result);

    if(result == Pl_Changed)
    {
        *host_frameendtime_computationduration = frameendtime_computationduration/flRatio;
        *host_frametime_stddeviation = frametime_stddeviation/flRatio;
        *host_framestarttime_stddeviation = framestarttime_stddeviation/flRatio;
    }
}


#ifdef _WIN32
DETOUR_DECL_STATIC0(HostRunFrame, void)
{
    DETOUR_STATIC_CALL(HostRunFrame)();
    OnFrameDataUpdated();
}
#else
DETOUR_DECL_STATIC1(HostRunFrame, uint32_t, uint32_t, arg0)
{
    int ret = DETOUR_STATIC_CALL(HostRunFrame)(arg0);
    OnFrameDataUpdated();
    return ret;
}
#endif 

bool NetInfoMgr::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
	char sConfError[256];
    if(!gameconfs->LoadGameConfigFile("netinfomgr.games", &g_pGameConfig, sConfError, sizeof(sConfError)))
    {
        snprintf(error, maxlen, "Could not read netinfomgr.games : %s", sConfError);
        return false;
    }

    g_pGameConfig->GetAddress("host_frameendtime_computationduration", (void**)&host_frameendtime_computationduration);
    if(!host_frameendtime_computationduration)
    {
        snprintf(error, maxlen, "Failed to get address of host_frameendtime_computationduration");
        return false;
    }

    g_pGameConfig->GetAddress("host_frametime_stddeviation", (void**)&host_frametime_stddeviation);
    if(!host_frametime_stddeviation)
    {
        snprintf(error, maxlen, "Failed to get address of host_frametime_stddeviation");
        return false;
    }

    g_pGameConfig->GetAddress("host_framestarttime_stddeviation", (void**)&host_framestarttime_stddeviation);
    if(!host_frameendtime_computationduration)
    {
        snprintf(error, maxlen, "Failed to get address of host_framestarttime_stddeviation");
        return false;
    }

    //Detour
    CDetourManager::Init(smutils->GetScriptingEngine(), g_pGameConfig);

    g_pHostRunFrame = DETOUR_CREATE_STATIC(HostRunFrame, "Host_RunFrame");
    if(!g_pHostRunFrame)
    {
        snprintf(error, maxlen, "Host_RunFrame detour could not be initialized ");
        return false;
    }
    g_pHostRunFrame->EnableDetour();

    //Forwards
    g_pOnFrameDataUpdatedForward = forwards->CreateForward("NIM_OnFrameDataUpdated", ET_Hook, 3, NULL, Param_FloatByRef, Param_FloatByRef, Param_FloatByRef);
    
    sharesys->RegisterLibrary(myself, "netinfomgr");
	return true;
}

void NetInfoMgr::SDK_OnUnload()
{
    gameconfs->CloseGameConfigFile(g_pGameConfig);

    if(g_pHostRunFrame)
        g_pHostRunFrame->DisableDetour();
}