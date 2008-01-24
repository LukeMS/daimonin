-- QUEST FUNCTIONS

require "quest_manager.lua"

print "WARNING: quest_check.lua is deprecated. Please convert your scripts to use quest_manager.lua"

-- return: 1=no quest, 2=quest is active, 3=solved and can be finished, 4=quest is finished & done
function Q_Status(_who, _qobj, _qstep, _lev, _skill, _qreq)
    print "WARNING: Q_Status is deprecated. See quest_manager.lua"
    local qm = QuestManager(_who, _qobj, _lev, _skill)
    if _qreq ~= nil then
        qm:AddRequiredQuest(_qreq)
    end
    return qm:GetStatus()
end

-- list the kills needed for this quest
function Q_List(_qobj, _ib)
    print "WARNING: Q_List is deprecated. See quest_manager.lua"
    return QuestManager(nil, _qobj):AddItemList(_ib)
end

-- remove kill quest items from the player
function Q_Remove(_qobj)
    print "WARNING: Q_Remove is deprecated. See quest_manager.lua"
    return QuestManager(nil, _qobj):RemoveQuestItems()
end
