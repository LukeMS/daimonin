-- test pedestals.lua

-- Tests pedestals with and without connections.
-- Gives a descriptive message to the player when a pedestal is triggered.

local pl = event.activator
local op = event.options

pl:Write(op)