-------------------------------------------------------------------------------
-- quest_builder.lua
-- 
-- High-level API for building quests.
-- Questbuilder (qb) is a sort of layer on top of questmanager (qm) and
-- must be used with qm (although the reverse is not true).
-- qb contains a sequential list of quests, 1-x.
-- The quests *must* be completed in order. There is no way to access
-- a higher number quest without first completing each lower number quest,
-- and it is impossible to have more than one quest active at a time.
-------------------------------------------------------------------------------
require("quest_manager")

QuestBuilder = { }

---------------------------------------
-- Meet... da management!
---------------------------------------
-------------------
-- qb:New() constructs a new, blank qb table (the return value).
-- Due to metatable magic you actually call, eg,
--     local qb = QuestBuilder()
-------------------
function QuestBuilder:New()
    local qb = { }
    setmetatable(qb, { __index = QuestBuilder })
    return qb
end

setmetatable(QuestBuilder, { __call = QuestBuilder.New })

-------------------
-- qb:AddQuest() adds an entry to qb.
-- Note that qb is not 'ready' until it is built with qb:Build().
-- _quest is the unique quest name. It must be a string.
-- _mode is the mode of that quest. It must be a number.
-- _level and _skill specify the level/skill combination which the player must
--   have to accept that quest. They must be numbers or nil.
-- _required is a list of other quest names which must be completed before this
--   quest can be accepted or nil. It must be a string, a table of strings, or
--   nil.
-- _finalstep is the number of the final step in a normal quest.
--   It is only relevant if _mode == game.QUEST_NORMAL and
--   must then be a number.
-------------------
function QuestBuilder:AddQuest(_quest, _mode, _level, _skill, _required, _finalstep)
    assert(type(_quest)     == "string",                                                    "Arg #1 must be string!")
    assert(type(_mode)      == "number",                                                    "Arg #2 must be number!")
    assert(type(_level)     == "number"                               or _level     == nil, "Arg #3 must be number or nil!")
    assert(type(_skill)     == "number"                               or _skill     == nil, "Arg #4 must be number or nil!")
    assert(type(_required)  == "string" or type(_required) == "table" or _required  == nil, "Arg #5 must be string or table or nil!")
    assert(type(_finalstep) == "number"                               or _finalstep == nil, "Arg #6 must be number or nil!")
    if _level == nil then
        _level = 1
    end
    if _skill == nil then
        _skill = game.ITEM_SKILL_NO
    end
    if _required ~= nil then
        if type(_required) ~= "table" then
            _required = { _required }
        end
        for i, v in ipairs(_required) do
            assert(type(v) == "string", "Arg #5, if a table, must be table of strings!")
        end
    end
    if _finalstep == nil then
        _finalstep = 1
    end

    table.insert(self, { name, mode, level, skill, required, finalstep })
    local n = table.getn(self)
    self[n].name      = _quest
    self[n].mode      = _mode
    self[n].level     = _level
    self[n].skill     = _skill
    self[n].required  = _required
    self[n].finalstep = _finalstep
end

-------------------
-- qb:Build() builds up the qb table based on the entries passed to it
-- previously by qb:AddQuest() so that qm can handle it.
-- _player is the player for whom the quest(s) will be registered.
--   It must be a player object.
-------------------
function QuestBuilder:Build(_player)
    assert(type(_player) == "GameObject" and _player.type == game.TYPE_PLAYER, "Arg #1 must be player GameObject!")

    local questnr
    for i, v in ipairs(self) do
        table.insert(self[i], 1, player)
        self[i].player = _player
        v.qm = QuestManager(v.player, v.name, v.level, v.skill) -- qm is a standard questmanager table for the quest
        if v.mode == game.QUEST_NORMAL and v.qm:GetStatus() == game.QSTAT_NO then v.qm:SetFinalStep(v.finalstep) end
        if type(v.required) == "table" then
            for j, w in ipairs(v.required) do v.qm:AddRequiredQuest(w) end
        end
        local qstat = v.qm:GetStatus()
        if questnr == nil and qstat ~= game.QSTAT_DISALLOW and qstat ~= game.QSTAT_DONE then questnr = i end
    end
    return questnr
end

-------------------
-- qb:GetQuestNr() returns the questnr of the named quest or the total number
-- of quests in qb if no name is given or 0 if the name given does not exist in
-- the qb.
-- _name a quest name (if a string) or nil.
-------------------
function QuestBuilder:GetQuestNr(_name)
    assert(type(_name) == "string" or _name == nil, "Arg #1 must be string or nil!")

    local questnr = table.getn(self)
    if type(_name) == "string" then
        while questnr >= 1 and not game:MatchString(self[questnr].name, _name) do questnr = questnr - 1 end
    end
    return questnr
end

-------------------
-- qb:GetName() returns the name of the quest qb[_nr].
-- _nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:GetName(_nr)
    assert(type(_nr) == "number", "Arg #1 must be number!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    return self[_nr].name
end

-------------------
-- qb:GetMode() returns the mode of the quest qb[_nr].
-- _nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:GetMode(_nr)
    assert(type(_nr) == "number", "Arg #1 must be number!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    return self[_nr].mode
end

-------------------
-- qb:GetStatus() returns the status of the quest qb[_nr].
-- _nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:GetStatus(_nr)
    assert(type(_nr) == "number", "Arg #1 must be number!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    return self[_nr].qm:GetStatus()
end

-------------------
-- qb:RegisterQuest() creates a new quest from qb[_nr] (qb[_nr].qm.trigger must
-- be nil), returning true if the quest could be added, false otherwise.
-- _nr is the quest in question. It must be a number.
-- _ib is the InterfaceBuilder which contains the quest description.
-------------------
function QuestBuilder:RegisterQuest(_nr, _ib)
    assert(type(_nr) == "number", "Arg #1 must be number!")
    assert(type(_ib) == "table",  "Arg #2 must be InterfaceBuilder!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    local success = self[_nr].qm:RegisterQuest(self:GetMode(_nr), _ib)
    assert(success, "Could not register quest!")
    local player = self[_nr].player
    player:Sound(0, 0, game.SOUND_LEARN_SPELL)
    player:Write("You start the quest '" .. self[_nr].name .. "'.", game.COLOR_NAVY)
    return success
end

-------------------
-- qb:IsRegistered() returns true if qb[_nr] is registered with the server.
-- _nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:IsRegistered(_nr)
    assert(type(_nr) == "number", "Arg #1 must be number!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    return self[_nr].qm:IsRegistered()
end

-------------------
-- qb:AddItemList() writes a 'checklist' for qb[_nr].
-- If _ib points to an InterfaceBuilder table it will append this list to
-- the message body portion of that table and also return a string version of
-- the list. If _ib is nil it will just return the string.
-- _nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:AddItemList(_nr, _ib)
    assert(type(_nr) == "number", "Arg #1 must be number!")
--    assert(type(_ib) == "table" or _ib == nil,  "Arg #2 must be InterfaceBuilder or nil!")
    assert(type(_ib) == "table",  "Arg #2 must be InterfaceBuilder!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    return self[_nr].qm:AddItemList(_ib)
end

-------------------
-- qb:AddQuestTarget() is a wrapper qm:AddQuestTarget() which is itself
-- a wrapper for object:AddQuestTarget()!
-- _nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:AddQuestTarget(_nr, ...)
    assert(type(_nr) == "number", "Arg #1 must be number!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    return self[_nr].qm:AddQuestTarget(unpack(arg))
end

-------------------
-- qb:AddQuestItem() is a wrapper for qm:AddQuestTarget() which is itself
-- a wrapper for object:AddQuestItem()!
-- _nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:AddQuestItem(_nr, ...)
    assert(type(_nr) == "number", "Arg #1 must be number!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    return self[_nr].qm:AddQuestItem(unpack(arg))
end

-------------------
-- qb:RemoveQuestItems() removes killitem or item quest items retrieved on
-- the quest qb[_nr] from the player's inventory.
-- _nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:RemoveQuestItems(_nr)
    assert(type(_nr) == "number", "Arg #1 must be number!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    return self[_nr].qm:RemoveQuestItems()
end

-------------------
-- qb:Finish() marks the quest qb[_nr] as finished.
-- _nr is the quest in question. It must be a number.
-- _reward is the reward player receives.
--   It must be nil (no reward message is printed) or
--   a string which is inserted into the following message: 'You were rewarded
--   with <_reward>!'
-------------------
function QuestBuilder:Finish(_nr, _reward)
    assert(type(_nr)     == "number",                   "Arg #1 must be number!")
    assert(type(_reward) == "string" or _reward == nil, "Arg #2 must be string or nil!")
    assert(_nr > 0 and _nr <= table.getn(self), "Not enough entries in qb table!")

    local player = self[_nr].player
    player:Sound(0, 0, game.SOUND_LEVEL_UP)
    player:Write("You finish the quest '" .. self[_nr].name .. "'.", game.COLOR_NAVY)
    if _reward then player:Write("You are rewarded with " .. _reward .. "!", game.COLOR_NAVY) end
    return self[_nr].qm:Finish()
end
