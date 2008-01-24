-- QUEST FUNCTIONS

QuestManager = {}

-- Create a new QuestManager for the given quest object and player
-- Mandatory parameters:
-- player: a player object or in some cases nil
-- quest: a quest object or a quest name or nil
-- Optional parameters:
-- level: require that the player is at least this level in the given skill
-- skill: require the given _lev in this skill to start the quest.
function QuestManager:New(player, quest, level, skill)
    local obj = {
        player = player, 
        quest_trigger = nil, 
        required = {}, 
        status = nil,
        name = nil,
        step = nil,
        end_step = nil
    }
   
    assert(player == nil or (type(player) == "GameObject" and player.type == game.TYPE_PLAYER), "player parameter must be nil or a player")
    
    assert(quest == nil or type(quest) == "string" or (type(quest) == "GameObject" and quest.type == game.TYPE_QUEST_TRIGGER), "quest parameter must be nil, a string or a quest trigger object")

    if type(quest) == "string" then
        obj.name = quest
        if player ~= nil then
            quest = player:GetQuest(quest)
        else
            quest = nil
        end
    elseif type(quest) == "GameObject" then
        obj.name = quest.name
    end
    obj.quest_trigger = quest
   
    obj.step = 0
    obj.end_step = 0

    -- Optional parameters
    if level == nil or skill == nil then
        obj.level = 1
        obj.skill = game.ITEM_SKILL_NO
    else 
        obj.level = level
        obj.skill = skill
    end

    setmetatable(obj, QuestManager._metatable)
    return obj
end

QuestManager._metatable = { __index = QuestManager }
setmetatable(QuestManager, { __call = QuestManager.New })

-- Get the status of the quest.
-- Will return one of: 
--   game.QSTAT_NO - Quest is not yet accepted, but ok to start
--   game.QSTAT_ACTIVE - Quest is started, but not finished
--   game.QSTAT_SOLVED - Quest is solved, but not rewarded
--   game.QSTAT_DONE - Quest is finished, rewarded and closed
--   game.QSTAT_DISALLOW - player lacks level or previous required quest.
function QuestManager:GetStatus()
    -- If there's no cached status value, create it
    if self.status == nil then
        self.status = self:_GetStatus(self.step, self.level, self.skill, self.required)
    end
    return self.status
end

-- _qstep: the quest step to check for (normally 0)
-- _lev: require that the player is at least this level in the given _skill
-- _skill: require the given _lev in this skill to start the quest.
-- required: table of names of quests that must be solved before this quest (or nil) 
function QuestManager:_GetStatus(_qstep, _lev, _skill, required)
    local _qobj = self.quest_trigger
    local _who = self.player
    
    assert(_who ~= nil, "player object must not be nil")

    if _qobj == nil then
        if required ~= nil then
            for _qreq in required do
                local _qr = _who:GetQuest(_qreq)
                if _qr == nil or _qr.last_eat ~= -1 then
                    return game.QSTAT_DISALLOW
                end
            end
        end
        if _who:CheckQuestLevel(_lev, _skill) == false then
            return game.QSTAT_DISALLOW
        end
        return game.QSTAT_NO
    end
    
    -- "step" is basically an unfinished server-side feature...
    if _qobj.magic < _qstep then
        return game.QSTAT_NO -- FIXME Really NO?
    elseif _qobj.last_eat == -1 then
        -- Already rewarded
        return game.QSTAT_DONE
    end

    if _qobj.sub_type_1 == game.QUEST_KILL then
        -- Check that all kills are done
        for _kcount in obj_inventory(_qobj) do
            if _kcount.last_sp > _kcount.level then
                return game.QSTAT_ACTIVE
            end
        end
    elseif _qobj.sub_type_1 == game.QUEST_KILLITEM then
        -- Check that all killitems are collected
        for _kcount in obj_inventory(_qobj) do
            if _kcount.inventory.quantity > _kcount:NrofQuestItem() then
                return game.QSTAT_ACTIVE
            end
        end
    elseif _qobj.sub_type_1 == game.QUEST_ITEM then
        -- Check that the quested item is retrieved
        for _kcount in obj_inventory(_qobj) do
            if _kcount.quantity > _kcount:NrofQuestItem() then
                return game.QSTAT_ACTIVE
            end
        end
    else
        -- Check generic quest status
        if _qobj.state ~= _qobj.magic then
            return game.QSTAT_ACTIVE
        end
    end
    return game.QSTAT_SOLVED
end

-- Write a "checklist" for the quest to the given InterfaceBuilder object
function QuestManager:AddItemList(_ib)
    local _ic
    local _qobj = self.quest_trigger

    _ib:AddMsg("\n|Quest Status|\n")
    if _qobj.sub_type_1 == game.QUEST_KILL then
        for _kcount in obj_inventory(_qobj) do
            if _kcount.last_sp > _kcount.level then
                _ib:AddMsg("\n  ".._kcount.name ..": " .. _kcount.level .. "/" .. _kcount.last_sp)
            else
                _ib:AddMsg("\n  ~".. _kcount.name ..": " .. _kcount.level .. "/" .. _kcount.last_sp .. " (complete)~")
            end
        end
    elseif _qobj.sub_type_1 == game.QUEST_ITEM then
        for _kcount in obj_inventory(_qobj) do
            _ic = _kcount:NrofQuestItem()
            if _kcount.quantity > _ic then
                _ib:AddMsg("\n  ".._kcount.name ..": " .. _ic .. "/" .. _kcount.quantity)
            else
                _ib:AddMsg("\n  ~".. _kcount.name ..": " .. _ic .. "/" .. _kcount.quantity .. " (complete)~")
            end
        end
    elseif _qobj.sub_type_1 == game.QUEST_KILLITEM then
        for _kcount in obj_inventory(_qobj) do
            _ic = _kcount:NrofQuestItem()
            if _kcount.inventory.quantity > _ic then
                _ib:AddMsg("\n  ".. _kcount.inventory.name ..": " .. _ic .. "/" .. _kcount.last_sp)
            else
                _ib:AddMsg("\n  ~".. _kcount.inventory.name ..": " .. _ic .. "/" .. _kcount.last_sp .. " (complete)~")
            end
        end
    else
        if _qobj.state == _qobj.magic then
            _ib:AddMsg("\n  ~Complete!~")
        else
            _ib:AddMsg("\n  Incomplete")
        end
    end
    _ib:AddMsg("\n")
end

-- Set the finish "step" for QUEST_NORMAL
function QuestManager:SetFinalStep(step)
    assert(not self:IsRegistered(), "The final step must be set up before registering")
    self.end_step = step
end

-- Add a quest that must be completed before this
-- Either give a name string, or another questmanager as parameter
function QuestManager:AddRequiredQuest(name_or_manager)
    if type(name_or_manager) == "string" then
        self.required[name_or_manager] = true
    else
        assert(getmetatable(name_or_manager) == QuestManager._metatable, "Must have a name string or QuestManager object as input")
        self.required[name_or_manager.name] = true
    end

    self.status = nil -- clear cache
end

-- Create a new quest for a QuestManager with a nil quest
-- mode: game.QUEST_KILLITEM etc
-- ib: InterfaceBuilder object that contains the quest description
-- returns: true if the quest could be added, false otherwise.
function QuestManager:RegisterQuest(mode, ib)
    assert(self.quest_trigger == nil, "This quest was already registered?")
    assert(self.name ~= nil, "The quest doesn't have a name.")

    -- Remove any clickable keywords from the interface. They won't work anyway.
    local qlist_text = string.gsub(ib:Build(), "%^", "")

    self.quest_trigger = self.player:AddQuest(self.name, mode, self.step, self.end_step, self.level, self.skill, qlist_text)
    self.status = nil -- clear cache

    return self.quest_trigger ~= nil
end

-- Return true if this manager's quest is registered with the server
function QuestManager:IsRegistered()
    return self.quest_trigger ~= nil
end

-- remove kill/collect quest items from the player's inventory
function QuestManager:RemoveQuestItems()
    assert(self.quest_trigger ~= nil, "The quest isn't registered")
    local mode = self.quest_trigger.sub_type_1
    if mode == game.QUEST_ITEM then
        self.quest_trigger:RemoveQuestItem()
    elseif mode == game.QUEST_KILLITEM then
        for obj in obj_inventory(self.quest_trigger) do
            obj:RemoveQuestItem()
        end
    else
        print("QuestManager: Sorry, don't know what to do about quest items from quest " .. self.quest_trigger.name)     
    end
end

-- Mark the quest as finished
-- You need to register the quest before calling this
function QuestManager:Finish()
    assert(self.quest_trigger ~= nil, "The quest isn't registered")
	self.quest_trigger:SetQuestStatus(-1)
    self.status = nil -- clear cache
end

-- Wrapper for object:AddQuestTarget()
-- You need to register the quest before calling this
-- Use this for QUEST_KILL or QUEST_KILLITEM quests
-- Returns a quest target object which you _must_ call AddQuestItem() on for KILLITEM quests
function QuestManager:AddQuestTarget(...)
    assert(self.quest_trigger ~= nil, "The quest isn't registered")
    self.status = nil -- clear cache
    return self.quest_trigger:AddQuestTarget(unpack(arg))
end

-- Wrapper for object:AddQuestItem()
-- You need to register the quest before calling this
function QuestManager:AddQuestItem(...)
    assert(self.quest_trigger ~= nil, "The quest isn't registered")
    self.status = nil -- clear cache
    return self.quest_trigger:AddQuestItem(unpack(arg))
end
