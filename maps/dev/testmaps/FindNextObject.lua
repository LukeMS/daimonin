-------------------------------------------------------------------------------
-- FindNextObject.lua
--
-- Very basic test of object:FindNextObject() core method.
--
-- TODO: Docs
-------------------------------------------------------------------------------
local fno = event.me
local player = event.activator

---------------------------------------
-- Find the object to start the search from. Either a marked inventory item or
-- player himself.
---------------------------------------
local nextobj = player:FindMarkedObject()

if nextobj == nil then
    nextobj = player
end

---------------------------------------
-- Do the search.
---------------------------------------
nextobj = nextobj:FindNextObject(game.TYPE_CONTAINER, game.FNO_MODE_CONTAINERS)

---------------------------------------
-- Print the results.
---------------------------------------
player:Write("Next object is " .. tostring(nextobj) .. "!", game.COLOR_RED)
