require("topic_list")
require("interface_builder")

local pl = event.activator
local me = event.me
local msg = string.lower(event.message)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name .. " the smith")

function repairCost(item)
    local cost = 5 + item:GetRepairCost()
    return cost
end

function topicDefault()
    ib:SetTitle("The Smithy")
    ib:SetMsg("Hello! I am " .. me.name .. ", the smith.\n")
    ib:AddMsg("i can repair or identify your equipment.\n\n")
    ib:AddMsg("What would you like to do?")
    ib:AddLink("Repair my Equipment", "repair")
    ib:AddLink("Identify Items", "identify")
    pl:Interface(1, ib:Build())
end

function topicRepair()
    local flag = false
    ib:SetTitle("Repair my Equipment")
    ib:SetMsg("Let me check your equipment...\nPerhaps an item needs a fix.\nI will tell you how much each will cost.")
    tmp = pl:FindMarkedObject()
    if tmp ~= nil and tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
        ib:AddLink("~*M*~ ".. tmp:GetName() .. '  ('.. tmp.item_condition .. '/' ..tmp.item_quality .. ')  costs: ' .. pl:ShowCost(repairCost(tmp), 1), "itemfix ".. -1)
        flag = true
    end
    for i=0,game.EQUIP_MAX-1 do
        tmp = pl:GetEquipment(i)
        if tmp ~= nil and tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
            ib:AddLink(tmp:GetName() .. '  ('.. tmp.item_condition .. '/' ..tmp.item_quality .. ')  costs: ' .. pl:ShowCost(repairCost(tmp), 1), "itemfix ".. i)
            flag = true
        end
    end
    if flag == false then
        ib:AddMsg("\n\n~Your equipment doesn't need any repair~")
    end
    ib:SetButton("Back", "hi") 
    pl:Interface(1, ib:Build())
end

function topicItemFix(what)
    local num = tonumber(what)
    if what=="-1" then 
        tmp = pl:FindMarkedObject()
    elseif num ~= nil then
        tmp = pl:GetEquipment(num)
    end
    if tmp == nil then
        topicRepair()
    else
        ib:SetTitle("Repairing")
        if tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
            ib:SetMsg("Will cost you " .. pl:ShowCost(repairCost(tmp),0))
            ib:AddMsg(".\n\nYou have " .. pl:ShowCost(pl:GetMoney()) .. ".\n\n") 
            ib:AddMsg("Should i repair it now?")
            ib:AddIcon(tmp:GetName(), tmp:GetFace(), 'Condition: ~'.. tmp.item_condition .. '~    Quality: ~' ..tmp.item_quality .. '~')
            ib:SetAccept("Repair", "fix " .. what)
            ib:SetDecline(nil, "repair") 
        else
            ib:SetMsg("The item doesn't need any repair.")
            ib:SetButton("Back", "repair") 
        end
        pl:Interface(1, ib:Build())
    end
end

function topicFix(what)
    local num = tonumber(what)
    if what=="-1" then 
        tmp = pl:FindMarkedObject()
    else
        if num == nil then
            topicRepair()
            return
        end
        tmp = pl:GetEquipment(num)
    end
    ib:SetTitle("Pay and Repair")
    if tmp == nil then
        ib:SetMsg("Hm, where is the item??")
    else
        if tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
            local qua = tmp.item_quality
            if pl:PayAmount(repairCost(tmp)) == 1 then
                tmp:Repair()
                ib:SetMsg("~** ".. me.name .." takes your money and the item **~\n")
                ib:AddMsg("~** after some time he returns **~\n\n")
                if tmp.item_quality < qua then
                    ib:AddMsg("Here is your repaired equipment!\nSadly it has lost a bit in quality!\nAnything else I can do for you?\n")
                else
                    ib:AddMsg("Here is your repaired equipment!\nThank you. Anything else I can do for you?\n")
                end
            else
                ib:SetMsg("You don't have enough money!")
            end
        else
            ib:SetMsg("The item doesn't need any repair.")
        end
        ib:AddIcon(tmp:GetName(), tmp:GetFace(), 'Condition: ~'.. tmp.item_condition .. '~    Quality: ~' .. tmp.item_quality .. '~')
    end
    ib:SetButton("Back", "repair") 
    pl:Interface(1, ib:Build())
end

function topicIdentify()
    ib:SetTitle("Item Identification")
    ib:SetMsg("Lets see what i can do for you.\nI can ~identify~ a single item or all.\nI can ~detect magic~ or ~detect curse~.\nRember you must mark the single item first.\n\n")
    ib:AddMsg(".You have " .. pl:ShowCost(pl:GetMoney()) .. ".\n\n") 
    ib:AddMsg("What would you like to do?\n")
    tmp = pl:FindMarkedObject()
    if tmp ~= nil then
        if tmp.f_identified ~= true then
            ib:AddLink("~*M*~ Identify ".. tmp:GetName() .. " for 150 copper", "detect single")
        else
            ib:AddMsg("\n~*M*~ Your marked item is already identified.")
        end
    end
    ib:AddLink("Identify all for 5 silver", "detect all")
    ib:AddLink("Detect magic for 50 copper", "detect magic")
    ib:AddLink("Detect curse for 50 copper", "detect curse")
    ib:SetButton("Back", "hi") 
    pl:Interface(1, ib:Build())
end

function topicDetect(what)
    ib:SetTitle("It will cost you")
    if what=="magic" then
        ib:SetMsg("I can cast ~Detect Magic~ for 50 copper")
    elseif what == "all" then
        ib:SetMsg("I can ~Identify all~ for 5 silver")
    elseif what == "curse" then
        ib:SetMsg("I can cast ~Detect Curse~ for 50 copper")
    else
        ib:SetMsg("I can ~Identify One Item~ for 150 copper")
    end
    ib:AddMsg(" coins.\n\nYou have " .. pl:ShowCost(pl:GetMoney()) .. ".\n\nYou want me to do it now?")
    ib:SetAccept(nil, "cast " .. what) 
    ib:SetDecline(nil, "identify") 
    pl:Interface(1, ib:Build())
end

function topicCast(what)
    if what == "magic" then
        sum = 50
        spell = "detect magic"
    elseif what == "curse" then
        sum = 50
        spell = "detect curse"
    elseif what == "all" then
        sum = 500
    else
        sum = 150
        mark = pl:FindMarkedObject(); 
        if mark == nil then
            topicIdentify()
            return
        end
    end
    ib:SetTitle("Identification...")
    if pl:PayAmount(sum) == 1 then
        if sum == 500 then
            me:IdentifyItem(pl, nil, game.IDENTIFY_ALL); 
        elseif sum == 150 then
            me:IdentifyItem(pl, mark, game.IDENTIFY_MARKED)
        else
            me:CastSpell(pl, game:GetSpellNr(spell), 1, 0, "")
        end
        ib:SetMsg("|**" .. me.name .. " takes your money **|\n\ndone!")
    else
        ib:SetMsg("You don't have enough money!")
    end
    ib:SetButton("Back", "identify") 
    pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("detect (.*)", topicDetect) 
tl:AddTopics("cast (.*)", topicCast) 
tl:AddTopics("repair", topicRepair) 
tl:AddTopics("itemfix (.*)", topicItemFix) 
tl:AddTopics("fix (.*)", topicFix)
tl:AddTopics("identify", topicIdentify) 
tl:CheckMessage(event)
