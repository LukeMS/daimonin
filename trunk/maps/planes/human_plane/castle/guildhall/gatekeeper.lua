-- gatekeeper.lua

-- triggered by:
-- 1. Gatehouse pedestals
--      checks players experience and calls him back to talk to Lothar
--      if a warning is needed.
--
-- 2. Talking to Lothar

require("topic_list")
require("quest_manager")
require("interface_builder")

local pl = event.activator  -- the player
local me = event.me         -- Lothar, or one of the pedestals

local ds = DataStore("guildhall_gatehouse", pl)
local spoken = ds:Get("spoken")

local ib = null
local tl = null

local req_level = 5         -- required level
local quest_list = {"Mouse Hunt", "The Mercenary Guild Quest", "Rusty rod"}
local quest_who  = {"Fanrir", "Cashin", "Jahrlen"}
local quest_max  = 3;

local text = null

local function checkQuests()
    local quest_num = 0
    for i = 1, quest_max do
        if QuestManager(pl, quest_list[i]):GetStatus() < game.QSTAT_DONE then
            break
        end
        quest_num = quest_num + 1
    end
    return quest_num
end

local function topicDefault()
    local q = checkQuests()
    ib:SetTitle("Lothar the Gatekeeper")

    if spoken then
        ib:SetMsg("\n\nWelcome back, friend!\n\n")
    else
        ds:Set("spoken", true)
        ib:SetMsg("\n\nHello, friend!\n\n")
    end
    
    if (q == quest_max) and (pl.level >= req_level) then
        ib:AddMsg("You are ready for the great adventures beyond these walls!\n\n")
        ib:AddMsg("Good luck, and may the tabernacle be with you on your travels.")
    else
        if spoken then
            if q < quest_max then
                ib:AddMsg("I see you've still not completed the important quests.\n")
                ib:AddMsg("I strongly advise you to go and talk to "..quest_who[q+1].." before you leave!")
            else
                ib:AddMsg("You are really not strong enough to leave yet. ")
                ib:AddMsg("It's a dangerous world out there!\n\n")
                ib:AddMsg("Go and kill a few more rats!")
            end
        else
            ib:AddMsg("A friendly word of advice: it's a dangerous world out there ")
            ib:AddMsg("beyond these walls.\n\n")
            if q == 0 then
                ib:AddMsg("The first thing you need to do is talk to Fanrir back there ")
                ib:AddMsg("by the lake. He will tell you what you need to do.")
            elseif q < quest_max then
                ib:AddMsg("There are several important quests you need to complete before ")
                ib:AddMsg("leaving. I advise you to go and talk to "..quest_who[q+1].." now.")
            else
                ib:AddMsg("Congratulations! You have completed the important quests.\n")
                ib:AddMsg("However, you should really get your strength up a bit before leaving.\n\n")
                ib:AddMsg("Go and kill a few more rats!")
            end
        end
    end
    pl:Interface(1, ib:Build())
end

-- check if this is a pedestal
if me.name == "pedestal" then
    -- only check if player is leaving the area
    if (me.weight_limit == 1) and
       (pl.direction >= game.SOUTHWEST and pl.direction <= game.NORTHWEST) then
        if (checkQuests() < quest_max) or (pl.level < req_level) then
            if spoken then
                text = "Don't say I didn't warn you!"
            else
                text = "Oi! Come back here! I need to talk to you!"
            end
            pl:Write("Lothar shouts: "..text, game.COLOR_YELLOW)
        end
    end
    
-- otherwise talking to Lothar
else
    ds:Set("spoken", true)
    ib = InterfaceBuilder()
    ib:SetHeader(me, me.name)
    tl = TopicList()
    tl:AddGreeting(nil, topicDefault)
    tl:SetDefault(topicDefault)
    tl:CheckMessage(event)
end