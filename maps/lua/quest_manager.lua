-------------------------------------------------------------------------------
-- quest_manager.lua | Utility
--
-- Deprecated for direct use, use quest_builder.lua.
-------------------------------------------------------------------------------
if QuestBuilder == nil then
    print("WARNING: quest_manager.lua is deprecated for direct use, use " ..
        "quest_builder.lua!")
end

---------------------------------------
-- If QuestManager already exists then check it is the correct addon type.
-- If it is not, this is an error. If it is, we (presumably) have already
-- required this chunk so no need to do it again.
---------------------------------------
if QuestManager ~= nil then
    local a, b, c = type(QuestManager)
    assert(a == "addon" and
        b == "utility" and
        c == "qm",
        "Global QuestManager exists but is not an addon utility qm!")
    return
end

---------------------------------------
-- Assign the global QuestManager table. Give it a metatable. The __call
-- metamethod means that calling QuestManager() returns a new instance of
-- the addon (as specified by the __metatable metamethod).
---------------------------------------
QuestManager = { total = 0, current = false }
setmetatable(QuestManager, {
    __call = function(self, player, quest, level, skill, repeats)
        assert(type(player) == "GameObject" and
               player.type == game.TYPE_PLAYER,
               "Arg #1 must be player GameObject!")
        assert(type(quest) == "string" or
               (type(quest) == "GameObject" and
                quest.type == game.TYPE_QUEST_TRIGGER),
               "Arg #2 must be string or quest trigger GameObject!")
        assert(type(level) == "number" or
               level == nil, "Arg #3 must be number or nil!")
        assert(type(skill) == "number" or
               skill == nil, "Arg #4 must be number or nil!")
        assert(type(repeats) == "number" or
               repeats == nil, "Arg #5 must be number or nil!")
        if level == nil then
            level = 1
        end
        if skill == nil then
            skill = game.ITEM_SKILL_NO
        end
        if repeats == nil then
            repeats = 0
        end

        ---------
        -- t is just a table so we give it a metatable. The __index event means
        -- that when we index t we treat it like QuestManager.
        ---------
        local t = {
            player = player,
            name,
            trigger,
            required = { },
            status,
            step = 0,
            end_step = 1,
            level = level,
            skill = skill,
            repeats = repeats
        }
        if type(quest) == "string" then
            t.name = quest
            t.trigger = player:GetQuest(quest)
        elseif type(quest) == "GameObject" then
            t.name = quest.name
            t.trigger = quest
        end
        setmetatable(t, {
            __index = QuestManager,
            __metatable = function() return "addon", "utility", "qm" end,
        })
        return t
    end,
    __metatable = function() return "addon", "utility", "qm" end
})

---------------------------------------
-- Methods
---------------------------------------
-------------------
-- qm:GetStatus() both sets qm.status and returns that value.
-------------------
function QuestManager:GetStatus()
    local function getstatus(qm)
        if qm.trigger == nil then
            if qm.required ~= nil then
                for v in qm.required do
                    local obj = qm.player:GetQuest(v)

                    if obj == nil or
                       obj.food == 0 then
                        return game.QSTAT_DISALLOW
                    end
                end
            end

            if qm.player:CheckQuestLevel(qm.level, qm.skill) == false then
                return game.QSTAT_DISALLOW
            else
                return game.QSTAT_NO
            end
        else
            if qm.trigger.environment.name == "QC: list done" then
                if qm.trigger.food == -1 then
                    return game.QSTAT_DONE
                else
                    return game.QSTAT_NO
                end
            end
            ---------
            -- For normal quests check generic quest status.
            ---------
            if qm.trigger.sub_type_1 == game.QUEST_NORMAL then
                if qm.trigger.state ~= qm.trigger.magic then
                    return game.QSTAT_ACTIVE
                end
            ---------
            -- For kill quests check that all kills are done.
            ---------
            elseif qm.trigger.sub_type_1 == game.QUEST_KILL then
                for obj in obj_inventory(qm.trigger) do
                    if obj.last_sp > obj.level then
                        return game.QSTAT_ACTIVE
                    end
                end
            ---------
            -- For killitem quests check that all killitems are collected.
            ---------
            elseif qm.trigger.sub_type_1 == game.QUEST_KILLITEM then
                for obj in obj_inventory(qm.trigger) do
                    if obj.inventory and
                       obj.inventory.quantity > obj:NrofQuestItem() then
                        return game.QSTAT_ACTIVE
                    end
                end
            ---------
            -- For item quests check that the quested item is retrieved.
            ---------
            else -- if qm.trigger.sub_type_1 == game.QUEST_ITEM then
                for obj in obj_inventory(qm.trigger) do
                    if obj.quantity > obj:NrofQuestItem() then
                        return game.QSTAT_ACTIVE
                    end
                end
            end
        end

        ---------
        -- If we get to here, the quest must be solved.
        ---------
        return game.QSTAT_SOLVED
    end

    if self.status == nil then
        self.status = getstatus(self)
    end

    return self.status
end

-------------------
-- qm:SetFinalStep() sets the finishing step required for a normal quest.
-------------------
function QuestManager:SetFinalStep(step)
    assert(type(step) == "number", "Arg #1 must be number!")
    assert(self.trigger == nil,
           "Final step must be set before registering quest!")

    self.end_step = step
end

-------------------
-- qm:AddRequiredQuest() adds a quest which must be completed before this one
-- can be.
-------------------
function QuestManager:AddRequiredQuest(quest)
    local a, b, c = type(quest)
    assert(a == "string" or
           (a == "addon" and b == "utility" and c =="qm") or
           (a == "GameObject" and quest.type == game.TYPE_QUEST_TRIGGER),
           "Arg #1 must be string, addon utility qm, or quest object!")

    if type(quest) == "string" then
        self.required[quest] = true
    else
        self.required[quest.name] = true
    end

    self.status = nil -- clear cache
end

-------------------
-- qm:RegisterQuest() creates a new quest from qm (qm.trigger must be nil),
-- returning true if the quest could be added, false otherwise.
-------------------
function QuestManager:RegisterQuest(qtype, ib)
    assert(type(qtype) == "number", "Arg #1 must be number!")
    local a, b, c = type(ib)
    assert(a == "addon" and
        b == "utility" and
        c == "ib", "Arg #2 must be addon utility ib!")
    assert(self.trigger == nil, "Quest already registered!")
    assert(type(self.name) == "string", "Quest doesn't have a name!")

    ---------
    -- Remove sound parameter from header tag.
    ---------
    if type(ib.header) == "table" then
        ib.header.sound = nil
    end

    ---------
    -- Temporary workaround.
    --
    -- Pre-SENTInce quests tended to use the message block (possibly as well as
    -- the description block) when registering a quest. This was partly to make
    -- things easier for the scripter, as they could just double-use the text
    -- they showed in the NPC talk interface, and partly because pre-SENTInce
    -- there was no strict and consistent approach in this area anyway. So
    -- people jst did whatever they preferred which did not help create a
    -- nice and consisten Daimonin look and feel for players.
    --
    -- Consistency is a central concept to SENTInce. Due to several
    -- enhancements a message block here is unnecessary and in fact will cause
    -- the qlist to fail with the INVALID GUI CMD errpr.
    --
    -- This workaround will prepend any existing message block to the
    -- description block (creating one first if needed) and delete the message
    -- block before writing the ib to the quest trigger. Which means the qlist
    -- will work again.
    --
    -- Eventually such scripts will be rewritten anyway, and this workaround
    -- will be removed at some point in B6.
    ---------
    if type(ib.message) == "table" then
        local tmp = ib.description

        ib.description = nil

        if ib.message.body ~= nil then
            ib:SetDesc(ib.message.body)
        end

        if type(tmp) == "table" then
            if tmp.body ~= nil then
                ib:AddDesc(tmp.body)
            end

            ib:SetCoins(tmp.copper, tmp.silver, tmp.gold, tmp.mithril)
        end

        ib.message = nil
    end

    ---------
    -- Remove message and button blocks from ib as these are useed internally
    -- by the server for qlists so if we also added them here, it would produce
    -- an error. The workaround above will already have transferred any message
    -- block to the description block.
    --
    -- Log a warning if anything was removed.
    ---------
    if ib.message ~= nil or
       ib.lhsbutton ~= nil or
       ib.rhsbutton ~= nil then
        local original = ib:Build()

        ib.message = nil
        ib.lhsbutton = nil
        ib.rhsbutton = nil
        game:Log(game.LOG_DEBUG,
                 "Interface string modified by qm:RegisterQuest()! This is " ..
                 "necessary but may result in yor quest missing info in " ..
                 "quest list. You should fix your script.")
        game:Log(game.LOG_DEBUG, "Original interface string: " .. original)
        game:Log(game.LOG_DEBUG, "Fixed interface string: " .. ib:Build())
    end

    ---------
    -- Remove any clickable keywords.
    ---------
    local text = string.gsub(ib:Build(), "%^", "")

    self.trigger = self.player:AddQuest(self.name, qtype, self.step,
                                        self.end_step, self.level,
                                        self.skill, text, self.repeats)
    self.status = nil -- clear cache

    return self.trigger ~= nil
end

-------------------
-- qm:IsRegistered() returns true if qm's quest is registered with the server.
-------------------
function QuestManager:IsRegistered()
    return self.trigger ~= nil
end

-------------------
-- qm:AddItemList() writes a 'checklist' for qm.
-- If ib points to an InterfaceBuilder table it will append this list, with a
-- leading and a trailing newline, to the message block body and also return
-- a string of the list. If ib is nil it will just return the string.
-------------------
function QuestManager:AddItemList(ib)
    local a, b, c = type(ib)
    assert((a == "addon" and b == "utility" and c == "ib") or
        ib == nil, "Arg #1 must be addon utility ib or nil!")

    local title, body = "|Quest Status:| ", ""
    local flag = true

    ---------
    -- For normal quests just report if it is complete or not.
    ---------
    if self.trigger.sub_type_1 == game.QUEST_NORMAL then
        if self.trigger.state ~= self.trigger.magic then
            flag = false
        end
    ---------
    -- For kill quests report the kills killed/kills required.
    ---------
    elseif self.trigger.sub_type_1 == game.QUEST_KILL then
        for obj in obj_inventory(self.trigger) do
            if obj.type ~= game.TYPE_QUEST_UPDATE then
                if obj.last_sp > obj.level then
                    body = body .. "\n  " .. obj.name .. ": ~" .. obj.level ..
                           "~/~" .. obj.last_sp .. "~"
                    flag = false
                else
                    body = body .. "\n  " .. obj.name .. ": ~" .. obj.level ..
                           "~/~" .. obj.last_sp .. "~ (|complete|)"
                end
            end
        end
    ---------
    -- For killitem quests report the killitems retrieved/killitems required.
    ---------
    elseif self.trigger.sub_type_1 == game.QUEST_KILLITEM then
        for obj in obj_inventory(self.trigger) do
            local quantity = obj:NrofQuestItem()

            if obj.type == game.TYPE_QUEST_INFO and
               obj.inventory then
                local qobj = obj.inventory

                if qobj.quantity > quantity then
                    body = body .. "\n  " .. qobj.name .. ": ~" .. quantity ..
                           "~/~" .. obj.last_sp .. "~"
                    flag = false
                else
                    body = body .. "\n  " .. qobj.name .. ": ~" .. quantity ..
                           "~/~" .. obj.last_sp .. "~ (|complete|)"
                end
            end
        end
    ---------
    -- For item quests report the items retrieved/items required.
    ---------
    else -- if self.trigger.sub_type_1 == game.QUEST_ITEM then
        for obj in obj_inventory(self.trigger) do
            local quantity = obj:NrofQuestItem()

            if obj.type ~= game.TYPE_QUEST_UPDATE then
                if obj.quantity > quantity then
                    body = body .. "\n  " .. obj.name .. ": ~" .. quantity ..
                           "~/~" .. obj.quantity .. "~"
                    flag = false
                else
                    body = body .. "\n  " .. obj.name .. ": ~" .. quantity ..
                    "~/~" .. obj.quantity .. "~ (|complete|)"
                end
            end
        end
    end

    ---------
    -- Overall, is the quest complete?
    ---------
    if flag == true then
        title = title .. "|Complete!|"
    else
        title = title .. "Incomplete"
    end

    ---------
    -- Possibly append the list to ib.message.body, and definitely return it.
    ---------
    if ib then
        ib:AddMsg("\n" .. title .. body .. "\n")
    end

    return title .. body
end

-------------------
-- qm:AddQuestTarget() is a wrapper for object:AddQuestTarget().
-------------------
function QuestManager:AddQuestTarget(chance, nrof, arch, name, race, title, level)
    assert(type(chance) == "number", "Arg #1 must be number!")
    assert(type(nrof) == "number", "Arg #2 must be number!")
    assert(type(arch) == "string" or
           arch == nil, "Arg #3 must be string or nil!")
    assert(type(name) == "string" or
           name == nil, "Arg #4 must be string or nil!")
    assert(type(race) == "string" or
           race == nil, "Arg #5 must be string or nil!")
    assert(type(title) == "string" or
           title == nil, "Arg #6 must be string or nil!")
    assert(type(level) == "number" or
           level == nil, "Arg #7 must be number or nil!")
    assert(self.trigger, "Quest not registered!")

    if level == nil then
        level = 0
    end

    self.status = nil -- clear cache

    return self.trigger:AddQuestTarget(chance, nrof, arch, name, race, title, level)
end

-------------------
-- qm:AddQuestItem() is a wrapper for object:AddQuestItem().
-------------------
function QuestManager:AddQuestItem(nrof, arch, face, name, title)
    assert(type(nrof) == "number", "Arg #1 must be number!")
    assert(type(arch) == "string", "Arg #2 must be string!")
    assert(type(face) == "string", "Arg #3 must be string!")
    assert(type(name) == "string" or
           name == nil, "Arg #4 must be string or nil!")
    assert(type(title) == "string" or
           title == nil, "Arg #5 must be string or nil!")
    assert(self.trigger, "Quest not registered!")

    self.status = nil -- clear cache

    return self.trigger:AddQuestItem(nrof, arch, face, name, title)
end

-------------------
-- qm:RemoveQuestItems() removes killitem or item quest items from the player's
-- inventory.
-------------------
function QuestManager:RemoveQuestItems()
    assert(self.trigger, "Quest not registered!")

    if self.trigger.sub_type_1 == game.QUEST_KILLITEM then
        for obj in obj_inventory(self.trigger) do
            obj:RemoveQuestItem()
        end
    elseif self.trigger.sub_type_1 == game.QUEST_ITEM then
        self.trigger:RemoveQuestItem()
    end
end

-------------------
-- qm:Finish() marks the quest as finished.
-- Quests keep the repeat count in food. This is handled automatically by the
-- server (common/quest.c set_quest_status()).
-------------------
function QuestManager:Finish()
    assert(self.trigger, "Quest not registered!")

    self.trigger.food = math.max(-1, self.trigger.food - 1)
    self.trigger:SetQuestStatus(self.trigger.food)
    self.status = nil -- clear cache
end
