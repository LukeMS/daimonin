-- Demonstrates how we can fetch and manipulate the behaviourlist of a mob.
-- Used by the woman in testmap_ai

local me = event.me
local activator = event.activator
local ai = me:GetAI()

require "behaviourlist.lua"

list = Behaviourlist(ai:GetBehaviourlist())

me:Say("My current behaviourlist:\n"..list:ToString())

if table.getn(list["moves"]) == 0 then
    -- If we have no movement behaviour
    me:Say("Adding a move_randomly behaviour")

    -- Insert at the end of the moves table, with parameters.
    table.insert(list["moves"], {name="move_randomly", parameters={xlimit={2}, ylimit={8}}})
    list:ReplaceAI(me)

    me:Say("My current behaviourlist:\n"..Behaviourlist(me:GetAI():GetBehaviourlist()):ToString())
elseif table.getn(list["moves"]) == 1 then
    -- If we have one movement behaviour
    me:Say("Inserting stand_still behaviour before move_randomly")

    -- Insert first in the moves table
    table.insert(list["moves"], 1, {name="stand_still", params={}})    
    list:ReplaceAI(me)

    me:Say("My current behaviourlist:\n"..Behaviourlist(me:GetAI():GetBehaviourlist()):ToString())
else
    me:Say("Sorry, no more tricks today")
end