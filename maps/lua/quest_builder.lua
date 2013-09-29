-------------------------------------------------------------------------------
-- quest_builder.lua | Utility
-- 
-- The quest builder is for building quests..
--
-- Qb is a sort of layer on top of the quest manager and must be used with qm
-- (although the reverse is not true). Qb loads qm if necessary internally.
--
-- Qb contains a sequential list of quests, 1-x. The quests *must* be completed
-- in order. There is no way to access a higher number quest without first
-- completing each lower number quest, and it is impossible to have more than
-- one quest active at a time.
-------------------------------------------------------------------------------
---------------------------------------
-- If QuestBuilder already exists then check it is the correct addon type.
-- If it is not, this is an error. If it is, we (presumably) have already
-- required this chunk so no need to do it again.
---------------------------------------
if QuestBuilder ~= nil then
    local a, b, c = type(QuestBuilder)
    assert(a == "addon" and
        b == "utility" and
        c == "qb",
        "Global QuestBuilder exists but is not an addon utility qb!")
    return
end

---------------------------------------
-- Assign the global QuestBuilder table. Give it a metatable. The __call
-- metamethod means that calling QuestBuilder() returns a new instance of
-- the addon (as specified by the __metatable metamethod).
---------------------------------------
QuestBuilder = { total = 0, current = false }
setmetatable(QuestBuilder, {
    __call = function()
        ---------
        -- t is just a table so we give it a metatable. The __index event means
        -- that when we index t we treat it like QuestBuilder.
        ---------
        local t = {}
        setmetatable(t, {
            __index = QuestBuilder,
            __metatable = function() return "addon", "utility", "qb" end,
        })
        return t
    end,
    __metatable = function() return "addon", "utility", "qb" end
})

require("quest_manager")

-------------------
-- qb:AddQuest() adds an entry to qb.
-- Note that qb is not 'ready' until it is built with qb:Build().
-------------------
function QuestBuilder:AddQuest(quest, mode, level, skill, required, finalstep, repeats, goal, reward, silent)
    assert(type(quest) == "string", "Arg #1 must be string!")
    assert(type(mode) == "number", "Arg #2 must be number!")
    assert(type(level) == "number" or
           level == nil, "Arg #3 must be number or nil!")
    assert(type(skill) == "number" or
           skill == nil, "Arg #4 must be number or nil!")
    assert(type(required) == "string" or
           type(required) == "table" or
           type(required) == "function" or
           required == nil, "Arg #5 must be string, table, function, or nil!")
    assert(type(finalstep) == "number" or
           finalstep == nil, "Arg #6 must be number or nil!")
    assert(type(repeats) == "number" or
           repeats == nil, "Arg #7 must be number or nil!")
    assert(type(goal) == "function" or
           goal == nil, "Arg #8 must be function or nil!")
    assert(type(reward) == "function" or
           reward == nil, "Arg #9 must be function or nil!")
    assert(type(silent) == "boolean" or
           silent == nil, "Arg #10 must be boolean or nil!")

    if level == nil then
        level = 1
    end

    if skill == nil then
        skill = game.ITEM_SKILL_NO
    end

    if type(required) == "table" then
        for i, v in ipairs(required) do
            assert(type(v) == "string",
                   "Arg #5, if a table, must be table of strings!")
        end
    elseif type(required) == "string" then
        required = { required }
    end

    if finalstep == nil then
        finalstep = 1
    end

    if repeats == nil then
        repeats = 0
    end

    self.total = self.total + 1

    table.insert(self, self.total, { ["name"] = quest, ["mode"] = mode,
                 ["level"] = level, ["skill"] = skill, ["required"] = required,
                 ["finalstep"] = finalstep, ["repeats"] = repeats,
                 ["goal"] = goal, ["reward"] = reward, ["silent"] = silent })
end

-------------------
-- qb:Build() builds up the qb table based on the entries passed to it
-- previously by qb:AddQuest() so that qm can handle it, and returns
-- a positive number which is the current quest that the player is on OR
-- the quest which is on offer and for which the player is eligible
-- (check qb:GetStatus(questnr)), or a negative number which indicates that
-- qb[math.abs(questnr)] is currently disallowed for this player
-- (eg, because of a level requirement), or 0 if the player has completed all
-- quests offered.
-------------------
function QuestBuilder:Build(player)
    assert(type(player) == "GameObject" and
           player.type == game.TYPE_PLAYER,
           "Arg #1 must be player GameObject!")
    assert(self.total >= 1, "No quests in qb table!")

    local nr = 0

    for i, v in ipairs(self) do
        table.insert(v, 1, player)
        v.player = player
        v.qm = QuestManager(v.player, v.name, v.level, v.skill, v.repeats)

        if type(v.required) == "table" then
            for _, w in ipairs(v.required) do
                v.qm:AddRequiredQuest(w)
            end
        end

        local qstat = v.qm:GetStatus()

        if qstat == game.QSTAT_ACTIVE or
            qstat == game.QSTAT_SOLVED then
            nr = i
        elseif nr == 0 and
            qstat ~= game.QSTAT_DONE then
            if type(v.required) == "function" then
                if qstat == game.QSTAT_NO and
                    v.required(i) == true then
                   nr = i
                end 
            elseif qstat == game.QSTAT_NO then
                nr = i
            elseif qstat == game.QSTAT_DISALLOW then
                nr = 0 - i
            end
        end
    end

    self.current = nr

    return nr
end

-------------------
-- qb:GetQuestNr() returns the nr of the named quest (negative if the quest is
-- disallowed) or 0 if the name given does not exist in the qb, or the total
-- number of quests in qb if name is nil or false, or the current quest nr if
-- name is true.
-------------------
function QuestBuilder:GetQuestNr(name)
    assert(type(name) == "string" or
           type(name) == "boolean" or
           name == nil, "Arg #1 must be string, boolean, or nil!")

    local nr

    if type(name) == "string" then
        assert(self.current ~= false, "Quest table not built, call qb:Build()!")
        assert(self.total >= 1, "No quests in qb table!")

        for i, v in ipairs(self) do
            if game:MatchString(v.name, name) then
                if v.qm:GetStatus() == game.QSTAT_DISALLOW then
                    nr = 0 - i
                else
                    nr = i
                end

                break
            end
        end
    elseif name == false or
           name == nil then
        nr = self.total
    else
        if self.current == false then
            nr = 0
        else
            nr = self.current
        end
    end

    return nr
end

-------------------
-- qb:GetName() returns the name of the quest qb[nr].
-------------------
function QuestBuilder:GetName(nr)
    assert(type(nr) == "number", "Arg #1 must be number!")
    assert(self.current ~= false, "Quest table not built, call qb:Build()!")
    nr = math.abs(nr)
    assert(nr >= 1 and
           nr <= self.total, "Not enough entries in qb table!")

    return self[nr].name
end

-------------------
-- qb:GetMode() returns the mode of the quest qb[nr].
-------------------
function QuestBuilder:GetMode(nr)
    assert(type(nr) == "number", "Arg #1 must be number!")
    assert(self.current ~= false, "Quest table not built, call qb:Build()!")
    nr = math.abs(nr)
    assert(nr >= 1 and
           nr <= self.total, "Not enough entries in qb table!")

    return self[nr].mode
end

-------------------
-- qb:GetStatus() returns the status of the quest qb[nr].
-------------------
function QuestBuilder:GetStatus(nr)
    assert(type(nr) == "number", "Arg #1 must be number!")
    assert(self.current ~= false, "Quest table not built, call qb:Build()!")
    nr = math.abs(nr)
    assert(nr >= 1 and
           nr <= self.total, "Not enough entries in qb table!")

    return self[nr].qm:GetStatus()
end

-------------------
-- qb:RegisterQuest() creates a new quest from qb[nr] (qb[nr].qm.trigger must
-- be nil), generating an error if registration fails.
-------------------
function QuestBuilder:RegisterQuest(nr, npc, ib)
    assert(type(nr) == "number", "Arg #1 must be number!")
    assert(type(npc) == "GameObject", "Arg #2 must be GameObject!")
    local a, b, c = type(ib)
    assert(a == "addon" and
        b == "utility" and
        c == "ib", "Arg #3 must be addon utility ib!")
    assert(self.current ~= false, "Quest table not built, call qb:Build()!")
    nr = math.abs(nr)
    assert(nr >= 1 and
           nr <= self.total, "Not enough entries in qb table!")

    local myib = InterfaceBuilder()

    myib:SetHeader(npc)

    if type(ib.description) == "table" then
        myib:SetDesc(ib.description.body, ib.description.copper,
                     ib.description.silver, ib.description.gold,
                     ib.description.mithril, ib.description.title)
    end

    if type(ib.icons) == "table" then
        for i, icon in ipairs(ib.icons) do
            myib:_AddIcon(icon.type, icon.title, icon.command, icon.face,
                          icon.body, icon.quantity, icon.mode)
        end

        myib:ActiveIcons(false)
    end

    assert(self[nr].qm:RegisterQuest(self[nr].mode, myib),
           "Could not register quest!")

    local player = self[nr].player

    player:Sound(0, 0, game.SOUND_LEARN_SPELL)
    if not self[nr].silent then
        player:Write("You start the quest '" .. self[nr].name .. "'.")
    end

    if type(self[nr].goal) == "function" then
        self[nr].goal(nr)
    end
end

-------------------
-- qb:IsRegistered() returns true if qb[nr] is registered with the server, or
-- false otherwise. If ib is an InterfaceBuilder, it also extracts the ib info
-- from the quest object and parses it back into ib, *overwriting any existing
-- data*.
-------------------
function QuestBuilder:IsRegistered(nr, ib)
    assert(type(nr) == "number", "Arg #1 must be number!")
    local a, b, c = type(ib)
    assert((a == "addon" and b == "utility" and c == "ib") or
        ib == nil, "Arg #2 must be addon utility ib or nil!")
    assert(self.current ~= false, "Quest table not built, call qb:Build()!")
    nr = math.abs(nr)
    assert(nr >= 1 and
           nr <= self.total, "Not enough entries in qb table!")

    local quest = self[nr].qm.trigger

    if ib ~= nil and
       quest ~= nil then
        ib:Unbuild(quest.message)
    end

    return quest ~= nil -- self[nr].qm:IsRegistered()
end

-------------------
-- qb:AddItemList() writes a 'checklist' for qb[nr].
-- If ib points to an InterfaceBuilder it will append this list, with a leading
-- and a trailing newline, to the message block body and also return a string
-- of the list. If ib is nil it will just return the string.
-------------------
function QuestBuilder:AddItemList(nr, ib)
    assert(type(nr) == "number", "Arg #1 must be number!")
    local a, b, c = type(ib)
    assert((a == "addon" and b == "utility" and c == "ib") or
        ib == nil, "Arg #2 must be addon utility ib or nil!")
    assert(self.current ~= false, "Quest table not built, call qb:Build()!")
    nr = math.abs(nr)
    assert(nr >= 1 and
           nr <= self.total, "Not enough entries in qb table!")

    return self[nr].qm:AddItemList(ib)
end

-------------------
-- qb:AddQuestTarget() is a wrapper for object:AddQuestTarget().
-------------------
function QuestBuilder:AddQuestTarget(nr, chance, nrof, arch, name, race, title, level)
    assert(type(nr) == "number", "Arg #1 must be number!")
    assert(type(chance) == "number", "Arg #2 must be number!")
    assert(type(nrof) == "number", "Arg #3 must be number!")
    assert(type(arch) == "string" or
           arch == nil, "Arg #4 must be string or nil!")
    assert(type(name) == "string" or
           name == nil, "Arg #5 must be string or nil!")
    assert(type(race) == "string" or
           race == nil, "Arg #6 must be string or nil!")
    assert(type(title) == "string" or
           title == nil, "Arg #7 must be string or nil!")
    assert(type(level) == "number" or
           level == nil, "Arg #8 must be number or nil!")
    assert(self.current ~= false, "Quest table not built, call qb:Build()!")
    nr = math.abs(nr)
    assert(nr >= 1 and
           nr <= self.total, "Not enough entries in qb table!")
    assert(self[nr].qm.trigger, "Quest not registered!")

    if level == nil then
        level = 0
    end

    self[nr].qm.status = nil -- clear cache

    return self[nr].qm.trigger:AddQuestTarget(chance, nrof, arch, name, race, title, level)
end

-------------------
-- qb:AddQuestItem() is a wrapper for object:AddQuestTarget().
-------------------
function QuestBuilder:AddQuestItem(nr, nrof, arch, face, name, title)
    assert(type(nr) == "number", "Arg #1 must be number!")
    assert(type(nrof) == "number", "Arg #2 must be number!")
    assert(type(arch) == "string", "Arg #3 must be string!")
    assert(type(face) == "string", "Arg #4 must be string!")
    assert(type(name) == "string" or
           name == nil, "Arg #5 must be string or nil!")
    assert(type(title) == "string" or
           title == nil, "Arg #6 must be string or nil!")
    assert(self.current ~= false, "Quest table not built, call qb:Build()!")
    nr = math.abs(nr)
    assert(nr >= 1 and
           nr <= self.total, "Not enough entries in qb table!")
    assert(self[nr].qm.trigger, "Quest not registered!")

    self[nr].qm.status = nil -- clear cache

    return self[nr].qm.trigger:AddQuestItem(nrof, arch, face, name, title)
end

-------------------
-- qb:Finish() marks the quest qb[nr] as finished.
-------------------
function QuestBuilder:Finish(nr, reward)
    assert(type(nr) == "number", "Arg #1 must be number!")
    assert(type(reward) == "string" or
           reward == nil, "Arg #2 must be string or nil!")
    assert(self.current ~= false, "Quest table not built, call qb:Build()!")
    nr = math.abs(nr)
    assert(nr >= 1 and
           nr <= self.total, "Not enough entries in qb table!")

    local player = self[nr].player

    player:Sound(0, 0, game.SOUND_LEVEL_UP)
    
    if not self[nr].silent then
        player:Write("You finish the quest '" .. self[nr].name .. "'.")
    end

    if reward then
        player:Write("You are rewarded with " .. reward .. "!")
    end

    if type(self[nr].reward) == "function" then
        self[nr].reward(nr)
    end

    return self[nr].qm:Finish()
end
