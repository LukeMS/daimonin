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
-- quest is the unique quest name. It must be a string.
-- mode is the mode of that quest. It must be a number.
-- level and skill specify the level/skill combination which the player must
--   have to accept that quest. They must be numbers or nil.
-- required is a list of other quest names which must be completed before this
--   quest can be accepted or nil. It must be a string, a table of strings, or
--   nil.
-- finalstep is the number of the final step in a normal quest.
--   It is only relevant if mode == game.QUEST_NORMAL and must then be a number.
-------------------
function QuestBuilder:AddQuest(quest, mode, level, skill, required, finalstep, goal, reward)
    assert(type(quest) == "string",
           "Arg #1 must be string!")
    assert(type(mode) == "number",
           "Arg #2 must be number!")
    assert(type(level) == "number" or
           level == nil,
           "Arg #3 must be number or nil!")
    assert(type(skill) == "number" or
           skill == nil,
           "Arg #4 must be number or nil!")
    assert(type(required) == "string" or
           type(required) == "table" or
           required  == nil,
           "Arg #5 must be string or table or nil!")
    assert(type(finalstep) == "number" or
           finalstep == nil,
           "Arg #6 must be number or nil!")
    assert(type(goal) == "function" or
           goal == nil,
           "Arg #7 must be function or nil!")
    assert(type(reward) == "function" or
           reward == nil,
           "Arg #8 must be function or nil!")
    if level == nil then
        level = 1
    end
    if skill == nil then
        skill = game.ITEM_SKILL_NO
    end
    if required ~= nil then
        if type(required) ~= "table" then
            required = { required }
        end
        for i, v in ipairs(required) do
            assert(type(v) == "string",
                   "Arg #5, if a table, must be table of strings!")
        end
    end
    if finalstep == nil then
        finalstep = 1
    end

    table.insert(self, { ["name"] = quest,
                         ["mode"] = mode,
                         ["level"] = level,
                         ["skill"] = skill,
                         ["required"] = required,
                         ["finalstep"] = finalstep,
                         ["goal"] = goal,
                         ["reward"] = reward })
end

-------------------
-- qb:Build() builds up the qb table based on the entries passed to it
-- previously by qb:AddQuest() so that qm can handle it, and returns
-- a positive number > 0 which is the current quest that the player is on OR
-- the quest which is on offer and for which the player is eligible
-- (check qb:GetStatus(questnr)), or a negative number which indicates that
-- qb[math.abs(questnr)] is currently disallowed for this player
-- (eg, because of a level requirement), or nil if the player has completed all
-- quests offered.
-- Calling this on an empty qb is an error.
-- player is the player for whom the quest(s) will be registered.
--   It must be a player object.
-------------------
function QuestBuilder:Build(player)
    assert(type(player) == "GameObject" and
           player.type == game.TYPE_PLAYER,
           "Arg #1 must be player GameObject!")

    assert(table.getn(self) >= 1,
           "No quests in qb table!")

    local questnr

    for i, v in ipairs(self) do
        table.insert(v, 1, player)
        v.player = player
        v.qm = QuestManager(v.player, v.name, v.level, v.skill)
        if v.mode == game.QUEST_NORMAL and v.qm:GetStatus() == game.QSTAT_NO then
             v.qm:SetFinalStep(v.finalstep)
        end
        if type(v.required) == "table" then
            for j, w in ipairs(v.required) do
                v.qm:AddRequiredQuest(w)
            end
        end

        local qstat = v.qm:GetStatus()

        if questnr == nil and qstat ~= game.QSTAT_DONE then
            if qstat == game.QSTAT_DISALLOW then
                questnr = 0 - i
            else
                questnr = i
            end
        end
    end

    return questnr
end

-------------------
-- qb:GetQuestNr() returns the questnr of the named quest (negative if
-- the quest is disallowed), or the total number of quests in qb if no name is
-- given, or 0 if the name given does not exist in the qb.
-- Calling this on an empty qb is an error.
-- name a quest name (if a string) or nil.
-------------------
function QuestBuilder:GetQuestNr(name)
    assert(type(name) == "string" or
           name == nil,
           "Arg #1 must be string or nil!")

    local questnr = table.getn(self)

    assert(questnr >= 1,
           "No quests in qb table!")

    if name == nil then
        return questnr
    end
    for i = questnr, 0, -1 do
        if questnr >= 1 and game:MatchString(self[questnr].name, name) then
            break
        end
    end

    if questnr >= 1 and self:GetStatus(questnr) == game.QSTAT_DISALLOW then
        return 0 - questnr
    else
        return questnr
    end
end

-------------------
-- qb:GetName() returns the name of the quest qb[nr].
-- nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:GetName(nr)
    assert(type(nr) == "number",
           "Arg #1 must be number!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")

    return self[nr].name
end

-------------------
-- qb:GetMode() returns the mode of the quest qb[nr].
-- nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:GetMode(nr)
    assert(type(nr) == "number",
           "Arg #1 must be number!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")

    return self[nr].mode
end

-------------------
-- qb:GetStatus() returns the status of the quest qb[nr].
-- nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:GetStatus(nr)
    assert(type(nr) == "number",
           "Arg #1 must be number!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")

    return self[nr].qm:GetStatus()
end

-------------------
-- qb:RegisterQuest() creates a new quest from qb[nr] (qb[nr].qm.trigger must
-- be nil), returning true if the quest could be added, false otherwise.
-- nr is the quest in question. It must be a number.
-- ib is the InterfaceBuilder which contains the quest description.
-------------------
function QuestBuilder:RegisterQuest(nr, ib)
    assert(type(nr) == "number",
           "Arg #1 must be number!")
    assert(type(ib) == "table",
           "Arg #2 must be InterfaceBuilder!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")
    local success = self[nr].qm:RegisterQuest(self[nr].mode, ib)
    assert(success,
           "Could not register quest!")

    local player = self[nr].player

    player:Sound(0, 0, game.SOUND_LEARN_SPELL)
    player:Write("You start the quest '" .. self[nr].name .. "'.")
    if type(self[nr].goal) == "function" then
        self[nr].goal(nr)
    end

    return success
end

-------------------
-- qb:IsRegistered() returns true if qb[nr] is registered with the server.
-- nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:IsRegistered(nr)
    assert(type(nr) == "number",
           "Arg #1 must be number!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")

    return self[nr].qm:IsRegistered()
end

-------------------
-- qb:AddItemList() writes a 'checklist' for qb[nr].
-- If ib points to an InterfaceBuilder table it will append this list to
-- the message body portion of that table and also return a string version of
-- the list. If ib is nil it will just return the string.
-- nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:AddItemList(nr, ib)
    assert(type(nr) == "number",
           "Arg #1 must be number!")
--    assert(type(ib) == "table" or
--           ib == nil,
--           "Arg #2 must be InterfaceBuilder or nil!")
    assert(type(ib) == "table",
           "Arg #2 must be InterfaceBuilder!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")

    return self[nr].qm:AddItemList(ib)
end

-------------------
-- qb:AddQuestTarget() is a wrapper qm:AddQuestTarget() which is itself
-- a wrapper for object:AddQuestTarget()!
-- nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:AddQuestTarget(nr, ...)
    assert(type(nr) == "number",
           "Arg #1 must be number!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")

    return self[nr].qm:AddQuestTarget(unpack(arg))
end

-------------------
-- qb:AddQuestItem() is a wrapper for qm:AddQuestTarget() which is itself
-- a wrapper for object:AddQuestItem()!
-- nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:AddQuestItem(nr, ...)
    assert(type(nr) == "number",
           "Arg #1 must be number!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")

    return self[nr].qm:AddQuestItem(unpack(arg))
end

-------------------
-- qb:RemoveQuestItems() removes killitem or item quest items retrieved on
-- the quest qb[nr] from the player's inventory.
-- nr is the quest in question. It must be a number.
-------------------
function QuestBuilder:RemoveQuestItems(nr)
    assert(type(nr) == "number",
           "Arg #1 must be number!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")

    return self[nr].qm:RemoveQuestItems()
end

-------------------
-- qb:Finish() marks the quest qb[nr] as finished.
-- nr is the quest in question. It must be a number.
-- reward is the reward player receives.
--   It must be nil (no reward message is printed) or
--   a string which is inserted into the following message: 'You are rewarded
--   with <reward>!'
-------------------
function QuestBuilder:Finish(nr, reward)
    assert(type(nr) == "number",
           "Arg #1 must be number!")
    assert(type(reward) == "string" or
           reward == nil,
           "Arg #2 must be string or nil!")

    nr = math.abs(nr)
    assert(nr > 0 and
           nr <= table.getn(self),
           "Not enough entries in qb table!")

    local player = self[nr].player

    player:Sound(0, 0, game.SOUND_LEVEL_UP)
    player:Write("You finish the quest '" .. self[nr].name .. "'.")
    if reward then
        player:Write("You are rewarded with " .. reward .. "!")
    end
    if type(self[nr].reward) == "function" then
        self[nr].reward(nr)
    end
    if self[nr].mode == game.QUEST_KILLITEM or self[nr].mode == game.QUEST_ITEM then
        self[nr].qm:RemoveQuestItems()
    end

    return self[nr].qm:Finish()
end
