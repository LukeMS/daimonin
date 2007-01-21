-- template for a "kill item quest" script
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)
-- quest names must be unique, the player will store the name forever
-- level and skill_group are optional
local level = 1
-- skill_group must be one of the 6 game item skill constants or use game.ITEM_SKILL_NO for main level
local skill_group = game.ITEM_SKILL_PHYSICAL
local q_mgr_1   = QuestManager(pl, "Dev Kill Item Test Quest", level, skill_group)
local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    if q_mgr_1:GetStatus() == game.QSTAT_DISALLOW then
        pl:Write(me.name .." tells you to come back when your weapon skills are better", game.COLOR_NAVY)
        pl:Interface(-1, "")
        return
    else
        if q_mgr_1:GetStatus() < game.QSTAT_DONE then
            ib:AddMsg("[DEVMSG] The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
            if q_mgr_1:GetStatus() == game.QSTAT_NO then
                ib:SetTitle("Kill Item Test Quest")
                ib:AddMsg("[INTRO] This quest is a mixture between kill and item quest: You have to kill defined mobs - instead of counting the kills you have a chance of x that you get a quest item from them.")
                ib:AddLink("Start Kill Item Test Quest", "startq1")
            else
                ib:SetTitle("Kill Item Test Quest solved?")
                ib:AddMsg("[pending] Have you done the quest?")
                ib:AddLink("Finish Kill Item Test Quest", "checkq1")
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
local function quest_icons1()
    ib:AddIcon("quest-reward-name", "shield.101", "i am the stats/description line") 
end
local function quest_body1()
    ib:SetMsg("[WHY] Get the item this test quest needs.")
    ib:SetDesc("[WHAT] Bring me °1 Slime Skins° and °2 Ogre Tooth°.\nHunt Ogres and Green Jelly for them.", 1, 2, 0, 0)
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
    if q_mgr_1:GetStatus() == game.QSTAT_NO then
        quest_body1()
        quest_icons1()
        if q_mgr_1:RegisterQuest(game.QUEST_KILLITEM, ib) then
            -- first target and item (0=drop always, 2=nrof we need)
            -- important: Add here the drop chance again - it overrules the one above
            q_mgr_1:AddQuestTarget(0, 2, "ogre", "Ogre"):
                    AddQuestItem(2, "quest_object", "tooth.101", "Ogre Tooth")
            -- second target and item (3=drop chance 1/3 = 30%, 1=nrof we need)
            q_mgr_1:AddQuestTarget(3, 1, "jelly_green", "Green Jelly"):
                    AddQuestItem(1, "quest_object", "skin.101", "Slime Skin") 
            pl:Sound(0, 0, 2, 0)
            pl:Write("You take the quest '".. q_mgr_1.name .."'.", game.COLOR_NAVY)
        end
        pl:Interface(-1, ib:Build()) 
    end
    topicDefault()
end

-- try to finish: check the quest
local function topCheckQ1()
    if q_mgr_1:GetStatus() == game.QSTAT_NO then
        topicDefault()
        return
    end
    ib:SetTitle("FINAL CHECK: Kill Item Test Quest")
    ib:SetMsg("[DEVMSG] The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
        ib:AddMsg("[not-done-text] Come back if you have killed them all!\n")
        ib:AddQuestChecklist(q_mgr_1)
        ib:SetButton("Back", "hi") 
    else
        ib:AddMsg("[final-text] Very well done! All is done.\n")
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
    ib:SetTitle("QUEST END: Kill Item Test Quest")
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
