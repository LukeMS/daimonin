-------------------------------------------------------------------------------
-- welcome.lua
--
-- A script to override magic mouths when approached from certain directions. In this way the mapmaker can make a welcoming message when a player enters a
-- building but not when he leaves.
--
-- Attach the scipt to the magic mouth as a trigger event. Set the script data to "<min>, <max>" where both are valid directions (single digits in the range
-- 1 to 8). Note that this delimits the range in which the player must *not* be facing.
-------------------------------------------------------------------------------
---------------------------------------
-- Parse the options.
---------------------------------------
local options = { string.find(event.options, "(%d)%s*,%s*(%d)") }; assert(options[1], "Insufficient options passed to script!")
local min     = tonumber(options[3])
local max     = tonumber(options[4])

---------------------------------------
-- Find out which way the player is facing.
---------------------------------------
local dir = event.activator.facing

---------------------------------------
-- Compare the two, overriding default behaviour (ie, don't trigger the magic mouth) if the player is not facing in the right direction.
---------------------------------------
if dir >= min and dir <= max then event.returnvalue = 1 end
