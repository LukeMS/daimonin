-------------------------------------------------------------------------------
-- examine_sign.lua
--
-- A script to make signs (and other readable scenery) respond to examine in the same way as apply, which is less confusing.
--
-- Attach this script to the readable scenery object as an examine event.
-------------------------------------------------------------------------------
local me        = event.me
local activator = event.activator

event.returnvalue = 1
activator:Apply(me, game.APPLY_ALWAYS)
