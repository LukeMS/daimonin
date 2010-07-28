--
-- Karamor the Merchant. Lvl: 5. Kill Thieves beneath the guildhall wilderness. Reward: Lvl 5 leggings. 
--

require("topic_list")
require("interface_builder")
require("quest_builder")

local pl = event.activator
local me = event.me
local qb = QuestBuilder()

function questGoal(questnr)
    qb:AddQuestItem(questnr, 1, "quest_object", "rubbish.101", "Karamor's Stolen Items")
    pl:Sound(0, 0, 2, 0)
    pl:Write("You take the quest '".. qb:GetName(questnr) .."'.", game.COLOR_NAVY)
end
function questReward(questnr)
    pl:CreateObjectInsideEx("leggings_leather", 1, 1) 
    pl:Sound(0, 0, 2, 0)
end
qb:AddQuest("Merchant's Trouble", game.QUEST_ITEM, nil, nil, nil, nil, 1,
            questGoal, questReward)

local questnr = qb:Build(pl)

local qstat  = qb:GetStatus(1)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name .. " the merchant")

local function quest1_body()
    ib:SetHeader("st_003", me)   
    ib:SetTitle(qb:GetName(questnr))
    ib:AddMsg("I have a big problem with break-ins.\n\n")
    ib:AddMsg("Someone has been breaking in and stealing equipment from my store for several nights now. Nobody has seen the thieves, but the guards have found hobgoblin tracks outside.\n\n")
    ib:AddMsg("If this keeps up I'll have to close down the shop! I need someone to track down the thieves, recover what has been stolen, and make sure they never do it again.\n\n")
    ib:AddMsg("The guards suspect hobgoblins, but they wouldn't be smart enough to sneak in here repeatedly. I even had ^Jahrlen^ put a warding spell on the shop, but it didn't help at all. If they are hobgoblins, they are really well organized.\n\n")
    ib:AddMsg("After the last break-in I followed the tracks into the eastern forest. The tracks just end right there near the northeastern corner of the guild hall compound. I couldn't figure out if they went over or under the wall.\n\n")
    ib:AddMsg("Be careful, though. These aren't any amateur shoplifters")
    ib:SetDesc("Find whoever is stealing from the merchant, kill them, and bring back the loot. I will reward you with something from my private collection.", 0,0,0,0)
    ib:AddIcon("Leather Leggins", "leggings_leather.101", "Well-made high leather boots")
end

function topicDefault()
    -- Autocomplete quest, if applicable
    if qstat == game.QSTAT_SOLVED then
        topicQuestComplete()
        return
    end
    ib:SetHeader("st_001", me)
    ib:SetTitle("Greetings")
    ib:SetMsg("Hello! I am " .. me.name .. ", the owner of this ^shop^.\n")

    if qstat == game.QSTAT_NO then
        ib:AddMsg("I'm having trouble with repeated break-ins. Could you help me, perhaps?\n\n")
        ib:AddLink("I'm interested", "explain quest")
    elseif qstat == game.QSTAT_ACTIVE then
        ib:AddMsg("So you still haven't found my items. I wonder if they are gone forever...")
    end
end

-- The player asks about available quests
function topicQuest()
    if qstat == game.QSTAT_ACTIVE then
        ib:SetHeader("st_003", me)
        ib:SetTitle(qb:GetName(questnr))
        ib:AddMsg("You still haven't found my items. I wonder if they are gone forever...")
    elseif qstat == game.QSTAT_NO then
        ib:SetHeader("st_003", me)
        quest1_body()
        ib:SetAccept("Accept", "accept quest")
    else
        topicGreeting()
        return
    end
end

-- The player wants to accept a quest. Activate the next accessible one.
function topicAccept()
    if qstat == game.QSTAT_NO then
        ib:SetHeader("st_003", me)
        quest1_body()
        qb:RegisterQuest(questnr, me, ib)
    else
        topicGreeting()
        return
    end
end

-- The player claims to have completed a quest. Double check and
-- possibly give out rewards
function topicQuestComplete(reward)
    ib:SetHeader("st_003", me)
    if qstat == game.QSTAT_ACTIVE then
        ib:SetTitle(qb:GetName(questnr))
        ib:AddMsg("You still haven't found my items. Are you trying to trick me?")
    elseif qstat == game.QSTAT_SOLVED then
        ib:SetMsg("Thank the Tabernacle! I never thought I'd see those things again\n\n")
        ib:AddMsg("Here, please take those boots as a reward:")
        ib:AddIcon("Leather Leggins", "leggings_leather.101", "Well-made high leather boots")
        ib:SetButton("Back", "hello")
        qb:Finish(questnr)
    else
        topicGreeting()
        return
    end
end

--
-- Rumors etc
--

local function topicShop()
    ib:SetHeader("st_002", me)
    ib:SetTitle("Shop")
    ib:SetMsg("\n\nYes, I'm trying to earn my living by supplying newcomers with high-quality equipment at premium prices.")
    ib:AddMsg("\n\nIf you want more specialized equipment you have to go to Stonehaven. But that can be a long and dangerous journey for the unexperienced.")
    ib:AddMsg("\n\nYou can enter the shop through the teleporter over there.")
    ib:AddMsg("\n\nIf you find something you like, just pick it up and exit through the teleporter on the other side. If you have enough money, you will be let through back automatically.")
    ib:AddMsg("\n\nIn case that you have something to sell, just drop it on the floor inside the fenced off area.")
    ib:SetButton("Back", "hi")
end

local function topicJahrlen()
    ib:SetHeader("st_004", me)
    ib:SetTitle("Jahrlen")
    ib:SetMsg("\n\nDon't you know him? He's the head mage of the mercenary guild. I guess he's better at offensive spells than he is at warding spells, otherwise I'd still have all of my inventory left.")
    ib:AddMsg("\n\nAnyhow, if you want to see him, I guess he is down in the guild.")
    ib:SetButton("Back", "hi")
end

tl = TopicList()
tl:SetDefault(topicDefault)

tl:AddTopics("jahrlen", topicJahrlen) 
tl:AddTopics("shop", topicShop) 

tl:AddTopics({"quest", "explain%s+quest"}, topicQuest)
tl:AddTopics({"accept", "accept%s+quest"}, topicAccept)
tl:AddTopics({"complete", "quest%s+complete%s*#?(%d*)"}, topicQuestComplete)

ib:ShowSENTInce(game.GUI_NPC_MODE_NPC, tl:CheckMessage(event, true))