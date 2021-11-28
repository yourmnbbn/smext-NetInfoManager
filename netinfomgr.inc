#if defined _netinfomgr_included
  #endinput
#endif
#define _netinfomgr_included

/**
 * Called when the frame data has been updated.
 *
 * @param host_frameendtime_computationduration         'sv' value of the server by reference.
 * @param host_frametime_stddeviation                   Deviation of 'sv' by reference.
 * @param host_framestarttime_stddeviation              'var' value of the server by reference.
 *          
 * @return                                              return Plugin_Changed to use the modified value.
 */
forward Action NIM_OnFrameDataUpdated(float& host_frameendtime_computationduration, float& host_frametime_stddeviation, float& host_framestarttime_stddeviation);


//Do not edit below this
public Extension __ext_fakequeries = 
{
	name = "netinfomgr",
	file = "netinfomgr.ext",
#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};