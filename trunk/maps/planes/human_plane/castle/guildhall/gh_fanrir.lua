-- Fanrir Quest - Guildhall Startquest 1 - gh_fanrir.lua
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)
-- quest names must be unique, the player will store the name forever
-- level and skill_group are optional
local level = 1
-- skill_group must be one of the 6 game item skill constants  or use game.ITEM_SKILL_NO for main level
local skill_group = game.ITEM_SKILL_NO
local q_mgr_1 = QuestManager(pl, "Examine Fanrir's Sack", level, skill_group)
local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    if q_mgr_1:GetStatus() == game.QSTAT_DISALLOW then
        pl:Write(me.name .." tells you to come back when your weapon skills are better", game.COLOR_NAVY)
        pl:Interface(-1, "") 
        return
    else
        if q_mgr_1:GetStatus() < game.QSTAT_DONE then
            ib:AddMsg("The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
            if q_mgr_1:GetStatus() == game.QSTAT_NO then
                ib:SetTitle("Welcome to Daimonin!")
                ib:AddMsg("I am Fanrir, the Guildhall Advisor.\n\nThis °Talk Interface° allows you to communicate with town people like me. You see the ~green line~ under this text? That is called a LINK LINE. It will lead you to a QUEST or an EVENT!\n\nClick on it with your mouse OR use the TAB key until the text is shown in purple in the bottom line and press then RETURN.")
                ib:AddLink("How to use the Talk Interface (TAB or mouse click)", "startq1")
            else
                ib:SetTitle("Examine Fanrir's Sack Quest")
                ib:AddMsg("[pending] Have you done the quest?")
                ib:AddLink("Finish Examine Fanrir's Sack Quest", "checkq1")
            end
        else
            pl:Write(me.name .." has nothing to say.", game.COLOR_NAVY)
            pl:Interface(-1, "") 
            return
        end
        pl:Interface(1, ib:Build())
    end
end
-- quest body (added to player quest obj for quest list)

local function quest_body1()
    ib:SetMsg("You have to learn how to use a container!\nA container is a chest, sack or bag. But also a shelf on a wall, a bookcase or a desk.\n\nNow, how do you open a container?\n\na.) move over the container you want open.\nb.) use the °cursor keys° to move the blue square over it\nc.) press the '°A°' key\n\nThat will open the container and show the items inside with a red border on the bottom. You can examine the items by moving the blue square with the cursor keys over them and pressing the '°E°' key.")
    ib:SetDesc("This is your first °Quest°! Open the sack next to me and examine the cape inside. Then '°T°'alk again to me.", 0, 0, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
    if q_mgr_1:GetStatus() ~= game.QSTAT_NO then
        topicDefault()
        return
    end
    ib:SetTitle(q_mgr_1.name)
    quest_body1()
    ib:SetAccept(nil, "acceptq1")
    ib:SetDecline(nil, "hi") 
    pl:Interface(1, ib:Build())
end

-- accepted: start the quest
local function topAcceptQ1()
    if q_mgr_1:GetStatus() ~= game.QSTAT_NO then
        topicDefault()
        return
    end
    quest_body1()
    --here we set the steps required to finish the quest
    q_mgr_1:SetFinalStep(1)
    if q_mgr_1:RegisterQuest(game.QUEST_NORMAL, ib) then
        pl:Sound(0, 0, 2, 0)
        pl:Write("You take the quest '"..q_mgr_1.name.."'.", game.COLOR_NAVY)
    end
    pl:Interface(-1, ib:Build())
end

-- try to finish: check the quest
local function topCheckQ1()
    if q_mgr_1:GetStatus() == game.QSTAT_NO then
        topicDefault()
        return
    end
    ib:SetTitle("Fanrir's Sack Quest")
    ib:SetMsg("The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
        ib:AddMsg("\nYou have not looked in the sack?!\n")
        ib:AddQuestChecklist(q_mgr_1)
        ib:SetButton("Back", "hi") 
    else
        ib:AddMsg("\nVery well done! You opened the sack and examined the contents!\n")
        ib:AddQuestChecklist(q_mgr_1)
        ib:SetAccept(nil, "finishq1") 
        ib:SetDecline(nil, "hi") 
    end
    pl:Interface(1, ib:Build())
end

-- done: finish quest and give reward
local function topFinishQ1()
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
        topicDefault()
        return
    end
    q_mgr_1:Finish()
    pl:Sound(0, 0, 2, 0)
    ib:SetTitle("You got it!")
    ib:SetMsg("Very well done! I see you have understood it.\nNow you are ready for a real quest.\n")
    ib:AddMsg("\n°And make sure that you apply the pillar next to me !°\n")
    ib:SetButton("Ok", "hi") 
    pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
if q_mgr_1:GetStatus() < game.QSTAT_DONE then
    tl:AddTopics("startq1", topStartQ1)
    tl:AddTopics("acceptq1", topAcceptQ1)
    tl:AddTopics("checkq1", topCheckQ1)
    tl:AddTopics("finishq1", topFinishQ1)
end
tl:CheckMessage(event)
