-- template for a "item quest" script
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)
-- quest names must be unique
local q_mgr_1   = QuestManager(pl,"Dev Item Test Quest")
local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    if q_mgr_1:GetStatus() < game.QSTAT_DONE then
        ib:AddMsg("[DEVMSG] The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
        if q_mgr_1:GetStatus() == game.QSTAT_NO then
            ib:SetTitle("Item Test Quest")
            ib:AddMsg("[INTRO] A quest which needs an item delivered. The item is inside the quest object inside the player which is given the player when this quest is accepted. There is a QUEST_TRIGGER inside a object (chest or mob) which will move a visible copy to the players inventory.")
            ib:AddLink("Start Item Test Quest", "startq1")
        else
            ib:SetTitle("Item Test Quest solved?")
            ib:AddMsg("[pending] You have done the quest?")
            ib:AddLink("Finish Item Test Quest", "checkq1")
        end
    else
        pl:Write(me.name .." has nothing to say.", game.COLOR_NAVY)
        pl:Interface(-1, "") 
        return
    end
    pl:Interface(1, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_icons1()
    ib:AddIcon("quest-reward-name", "shield.101", "i am the stats/description line") 
end
local function quest_body1()
    ib:SetMsg("[WHY] Get the item this test quest needs.")
    ib:SetDesc("[WHAT] Bring me a °Item Test Helm°.\nKill the blob or open the chest.", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
    if q_mgr_1:GetStatus() ~= game.QSTAT_NO then
        topicDefault()
        return
    end		
    ib:SetTitle(q_mgr_1.name)
    quest_body1()
    quest_icons1()
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
    quest_icons1()
    if q_mgr_1:RegisterQuest(game.QUEST_ITEM, ib) then
        q_mgr_1:AddQuestItem(1, "quest_object", "helm_leather.101", "Item Test Helm")
        pl:Sound(0, 0, 2, 0)
        pl:Write("You take the quest '".. q_mgr_1.name .."'.", game.COLOR_NAVY)
    end
    pl:Interface(-1, ib:Build())
end

-- try to finish: check the quest
local function topCheckQ1()
    if q_mgr_1:GetStatus() == game.QSTAT_NO then
        topicDefault()
        return
    end
    ib:SetTitle("FINAL CHECK: Item Test Quest")
    ib:SetMsg("[DEVMSG] The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
        ib:AddMsg("[not-done-text] Come back if you have it!\n")
        ib:AddQuestChecklist(q_mgr_1)
        ib:SetButton("Back", "hi") 
    else
        ib:AddMsg("[final-text] Very well done! You found the helm.\n")
        ib:SetDesc("here it is...", 1, 2, 0, 0)
        quest_icons1()
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
    q_mgr_1:RemoveQuestItems()
    q_mgr_1:Finish()
    pl:Sound(0, 0, 2, 0)
    pl:CreateObjectInsideEx("shield", 1,1)
    pl:AddMoneyEx(1,2,0,0)
    ib:SetTitle("QUEST END: Item Test Quest")
    ib:SetMsg("Very well done! Here is your reward!")
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
