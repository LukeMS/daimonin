-- QUEST FUNCTIONS

-- return: 1=no quest, 2=quest is active, 3=solved and can be finished, 4=quest is finished & done
function Q_Status(_who, _qobj, _qstep, _lev, _skill, _qreq)
    if _qobj == nil then
        if _qreq ~= nil then
            local _qr = _who:GetQuest(_qreq)
            if _qr == nil or _qr.last_eat ~= -1 then
                return 5
            end
        end
        if _who:CheckQuestLevel(_lev, _skill) == 0 then
            return 5
        end
        return 1
    end
    if _qobj.magic < _qstep then
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
    elseif _qobj.sub_type_1 == game.QUEST_KILLITEM then
        while (_kcount ~= nil) do
            if _kcount.inventory.quantity > _kcount:NrofQuestItem() then
                return 2
            end
            _kcount = _kcount.below
        end
    elseif _qobj.sub_type_1 == game.QUEST_ITEM then
        while (_kcount ~= nil) do
            if _kcount.quantity > _kcount:NrofQuestItem() then
                return 2
            end
            _kcount = _kcount.below
        end
    else
        if _qobj.state ~= _qobj.magic then
            return 2
        end
    end
    return 3
end

-- list the kills needed for this quest
function Q_List(_qobj, _ib)
    local _ic
    local _kcount = _qobj.inventory
    _ib:AddMsg("\n°STATUS:°")
    if _qobj.sub_type_1 == game.QUEST_KILL then
        while (_kcount ~= nil) do
            if _kcount.last_sp > _kcount.level then
                _ib:AddMsg("\n".. _kcount.name ..": " .. _kcount.level .. "/" .. _kcount.last_sp)
            else
                _ib:AddMsg("\n°".. _kcount.name ..": " .. _kcount.level .. "/" .. _kcount.last_sp .. " (complete)°")
            end
            _kcount = _kcount.below
        end
    elseif _qobj.sub_type_1 == game.QUEST_ITEM then
        while (_kcount ~= nil) do
            _ic = _kcount:NrofQuestItem()
            if _kcount.quantity > _ic then
                _ib:AddMsg("\n".. _kcount.name ..": " .. _ic .. "/" .. _kcount.quantity)
            else
                _ib:AddMsg("\n°".. _kcount.name ..": " .. _ic .. "/" .. _kcount.quantity .. " (complete)°")
            end
            _kcount = _kcount.below
        end
    elseif _qobj.sub_type_1 == game.QUEST_KILLITEM then
        while (_kcount ~= nil) do
            _ic = _kcount:NrofQuestItem()
            if _kcount.inventory.quantity > _ic then
                _ib:AddMsg("\n".. _kcount.inventory.name ..": " .. _ic .. "/" .. _kcount.last_sp)
            else
                _ib:AddMsg("\n°".. _kcount.inventory.name ..": " .. _ic .. "/" .. _kcount.last_sp .. " (complete)°")
            end
            _kcount = _kcount.below
        end
    else
        if _qobj.state == _qobj.magic then
            _ib:AddMsg("\n°Complete!°")
        else
            _ib:AddMsg("\nIncomplete")
        end
    end
end

-- remove kill quest items from the player
function Q_Remove(_qobj)
    local _kcount = _qobj.inventory
    while (_kcount ~= nil) do
        _kcount:RemoveQuestItem()
        _kcount = _kcount.below
    end
end
