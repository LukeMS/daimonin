-------------------------------------------------------------------------------
-- remote_connection.lua
--
-- A script to handle triggering a connection on another map. Eg, to open a gate on map B by flicking a switch on map A.
--
-- On the first map is a switch with this script attached as an apply event. On the second map is a second switch (the trigger) with a beacon in it.
-- This second switch is connected to, eg, a gate on the same map.
-- 
-- When player applies the first switch, it triggers the script. The script loads the second map into memory if it is not already and finds the beacon (and
-- therefore the second switch). Then it causes the second switch to apply itself, which activates the gate.
--
-- Usually the second switch should be:
--     layer 0
--     sys_object 1
-- so that it is undetectable to players, but if you want the second switch to also be directly useable, this is unnecessary.
--
-- The first switch should have its script data set to:
--     <absolute path to second map> | <unique beacon name>
-- Eg, "/planes/human_plane/somedungeon/somemap_0000|somedungeon secret door control"
--
-- It is possible to have many remote controls all triggering the same connection.
-------------------------------------------------------------------------------
---------------------------------------
-- Parse the options into mapname and beaconname.
---------------------------------------
local options = { string.find(event.options, "(.+)%s*|%s*(.+)") }; assert(options[1], "Insufficient options passed to script!")
local mapname    = options[3]
local beaconname = options[4]

---------------------------------------
-- Ready the second map. This makes sure it is in memory, loading it if necessary.
---------------------------------------
game:ReadyMap(mapname)

---------------------------------------
-- Locate the beacon and by extension the second switch.
---------------------------------------
local beacon  = game:LocateBeacon(beaconname); assert(beacon,  "Could not find beacon!")
local trigger = beacon.environment;            assert(trigger, "Could not find trigger!")

---------------------------------------
-- Activate the trigger.
---------------------------------------
if trigger:Apply(trigger, game.APPLY_TOGGLE) == 0 then error("Could not activate trigger!") end
