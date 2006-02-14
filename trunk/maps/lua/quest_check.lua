-- KILL & KILL ITEM QUEST FUNCTIONS

-- return: 1=no quest, 2=quest is active, 3=solved and can be finished, 4=quest is finished & done
function QKill_Status(_qobj, _qstep)
if _qobj == nil or _qobj.magic < _qstep then
return 1
elseif _qobj.last_eat == -1 then
return 4
end
local _kcount = _qobj.inventory
if _qobj.sub_type_1 == game.QUEST_KILL then
while (_kcount ~= nil) do
if _kcount.last_sp > _kcount.level then
return 2
end
_kcount = _kcount.below
end
else
while (_kcount ~= nil) do
if _kcount.last_sp > _kcount:NrofKillQuestItem() then
return 2
end
_kcount = _kcount.below
end
end
return 3
end

-- list the kills needed for this quest
function QKill_List(_qobj, _ib)
local _kcount = _qobj.inventory
if _qobj.sub_type_1 == game.QUEST_KILL then
while (_kcount ~= nil) do
_ib:AddToMessage("\nYou has done °" .. _kcount.level .. "° of °" .. _kcount.last_sp .. " ".. _kcount.name .."°.")
_kcount = _kcount.below
end
else
while (_kcount ~= nil) do
_ib:AddToMessage("\nYou has done °" .. _kcount:NrofKillQuestItem() .. "° of °" .. _kcount.last_sp .. " ".. _kcount.inventory.name .."°.")
_kcount = _kcount.below
end
end
end

-- remove kill quest items from the player
function QKill_Remove(_qobj)
local _kcount = _qobj.inventory
while (_kcount ~= nil) do
_kcount:RemoveKillQuestItem()
_kcount = _kcount.below
end
end