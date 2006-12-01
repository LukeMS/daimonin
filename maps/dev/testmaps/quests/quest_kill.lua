-- template for a "kill quest" script
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)
local q_mgr_1   = QuestManager(pl, "Dev Kill Test Quest")

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    if q_mgr_1:GetStatus() < game.QSTAT_DONE then
        ib:AddMsg("[DEVMSG] The quest status is: "..q_mgr_1:GetStatus().."\n\n")
        if q_mgr_1:GetStatus() == game.QSTAT_NO then
            ib:SetTitle("Kill Test Quest")
            ib:AddMsg("[INTRO] This kind of quest is solved when you have killed a given number of different mobs.")
            ib:AddLink("Start Kill Test Quest", "startq1")
        else
            ib:SetTitle("Kill Test Quest solved?")
            ib:AddMsg("[pending] You have done the quest?")
            ib:AddLink("Finish Kill Test Quest", "checkq1")
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
    ib:SetDesc("[WHAT] Kill 1 Red Ant and 2 Kobolds.", 1, 2, 0, 0)
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
    if q_mgr_1:RegisterQuest(game.QUEST_KILL, ib) then
        -- add the targets: chance (0=always), nrof to kill, mob definition
        q_mgr_1:AddQuestTarget(0, 1, "ant_red", "Red Ant", "Big Red Ant" )
        q_mgr_1:AddQuestTarget(0, 2, "kobold", "Kobold")

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
    ib:SetTitle("FINAL CHECK: Kill Test Quest")
    ib:SetMsg("[DEVMSG] The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
        ib:AddMsg("[not-done-text] Come back when you have killed them all!\n")
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
    q_mgr_1:Finish()
    pl:Sound(0, 0, 2, 0)
    pl:CreateObjectInsideEx("shield", 1,1)
    pl:AddMoneyEx(1,2,0,0)
    ib:SetTitle("QUEST END: Kill Test Quest")
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
