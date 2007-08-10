-------------------------------------------------------------------------------
-- welcome.lua
--
-- A script to override magic mouths when approached from certain directions. In this way the mapmaker can make a welcoming message when a player enters a
-- building but not when he leaves.
--
-- Attach the scipt to the magic mouth as a trigger event. Set the script data to a list of valid directions (single digits in the range 1 to 8 -- any other
-- character(s) or none can be used as serparators). If player is facing in one of those directions the message will be shown. Otherwise it won't.
-------------------------------------------------------------------------------
---------------------------------------
-- By default the script will override the magic mouth (no message will be shown).
---------------------------------------
event.returnvalue = 1

---------------------------------------
-- If the direction in which player is facing is found in the script data, unset event.retunvalue.
---------------------------------------
if string.find(event.options, tostring(event.activator.facing)) then event.returnvalue = 0 end
