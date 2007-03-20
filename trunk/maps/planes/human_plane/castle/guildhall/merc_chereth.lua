-- merc_chereth water well ant queen head quest
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local q_mgr_1   = QuestManager(pl, "Water Well Ant Queen")

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)
local skill_bow = game:GetSkillNr("bow archery")
local skill_cbow = game:GetSkillNr("crossbow archery")
local skill_sling = game:GetSkillNr("sling archery")

local function topicDefault()
    if pl:FindSkill(skill_bow) ~= nil or pl:FindSkill(skill_cbow) ~= nil or pl:FindSkill(skill_sling) ~= nil then
        pl:Write(me.name .." has nothing more to teach you.", game.COLOR_NAVY)
        pl:Interface(-1, "")
        return
    end
    if q_mgr_1:GetStatus() < game.QSTAT_DONE then
        ib:AddMsg("The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
        if q_mgr_1:GetStatus() == game.QSTAT_NO then
            ib:SetTitle("Supply Chief Chereth")
            ib:AddMsg("\nHello, mercenary. I am Supply Chief Chereth.")
            ib:AddMsg("\nFomerly Archery Commander Chereth,\nbefore I lost my ^eyes^.")
            ib:AddMsg("\nWell, I still know alot about ^archery^ and I can train you.")


        else
            ib:SetTitle("The Water Well Ant Queen is Dead?")
            ib:AddMsg("You have completed the quest?")
            ib:AddLink("Finish Water Well Ant Queen Quest", "checkq1")
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
    ib:AddSelect("#1 Learn Archery skill of Sling ", "sling_small.101", " ")
    ib:AddSelect("#2 Learn Archery skill of Bow ", "bow_short.101", " ")
    ib:AddSelect("#3 Learn Archery skill of Crossbow ", "crossbow_small.101", " ")
end

local function quest_body1()
    ib:SetMsg("We have to fix the water well")
    ib:SetDesc("Bring me the °head of the water well ant queen °.\nShe is in the well, east of the guild.", 0, 0, 0, 0)
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
    ib:SelectOff()
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
    ib:SelectOff()
    if q_mgr_1:RegisterQuest(game.QUEST_ITEM, ib) then
        q_mgr_1:AddQuestItem(1, "quest_object", "head_ant_queen.101", "Head of the Ant Queen")
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
    ib:SetTitle("Water Well Ant Queen Quest")
    ib:SetMsg("The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
        ib:AddMsg("Come back if you have it!\n")
        ib:AddQuestChecklist(q_mgr_1)
        ib:SetButton("Back", "hi") 
    else
        ib:AddMsg("Very well done! You have the head!.\n")
        ib:SetDesc("here it is...", 0, 0, 0, 0)
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
    local words = string.split(event.message)
    q_mgr_1:RemoveQuestItems()
    q_mgr_1:Finish()
    pl:Sound(0, 0, 2, 0)
    if words[2]=="#1" then  
        pl:AcquireSkill(skill_sling, game.LEARN)
        pl:CreateObjectInside("sling_small", 1,1)
		pl:Write("Chereth gives you a small sling.", game.COLOR_WHITE)
		pl:CreateObjectInside("sstone", 1,12)
		pl:Write("Chereth gives you 12 sling stones.", game.COLOR_WHITE)
    elseif words[2]=="#2" then  
        pl:AcquireSkill(skill_bow, game.LEARN)
        pl:CreateObjectInside("bow_short", 1,1)
		pl:Write("Chereth gives you a short bow.", game.COLOR_WHITE)
		pl:CreateObjectInside("arrow", 1,12)
		pl:Write("Chereth gives you 12 arrows.", game.COLOR_WHITE)
    else
        pl:AcquireSkill(skill_cbow, game.LEARN)
        pl:CreateObjectInside("crossbow_small", 1,1)
		pl:Write("Chereth gives you a small crossbow.", game.COLOR_WHITE)
		pl:CreateObjectInside("bolt", 1,12)
		pl:Write("Chereth gives you 12 bolts.", game.COLOR_WHITE)
    end
    ib:SetTitle("QUEST END: Water Well Ant Queen")
    ib:SetMsg("Very well done! Here is your reward!")
    ib:SetButton("Ok", "hi") 
    pl:Interface(-1, ib:Build())
end

local function topQuest()
    ib:SetTitle("Chereth's Problem")
    ib:SetMsg("\nYes, we need your help first.\n")
    ib:AddMsg("As supply chief the water support of this outpost is under my command. \n")
    ib:AddMsg("We noticed the last few days problems with our main water source.\n")
    ib:AddMsg("It seems a traveling hive of giant ants has invaded the caverns under our water well.\n")
    ib:AddMsg("Enter the well east of here, by the smith and kill the ant queen!\n")
    ib:AddMsg("Bring me her head as a trophy and then I will teach you an archery skill.")
    ib:AddLink("Start Water Well Ant Queen Quest", "startq1")
    ib:SetButton("Ok", "hi")
    pl:Interface(1, ib:Build())
end

local function topEyes()
    ib:SetTitle("Chereth's Last Battle")
    ib:SetMsg("\n   Investigating a report of a magical disturbance in the area, my group and I found a freshly broken hole int the back wall of a wyvern cave.\n")
    ib:AddMsg("Venturing through the hole we came upon a large room filled with Fire Wyverns and something worse.\n")
    ib:AddMsg("\n   Fire Wyvern Lords are rare, but this was an extremely rare and powerful Fire Wyvern High Lord!\n")
    ib:AddMsg("Our mage and our cleric cast spell after spell, our swordsmen danced with their sword around him whilst I, with my arrows, did pepper his hide.\n")
    ib:AddMsg("\n   After several hours of battle, the beast finally show signs of weakening, so I loaded my special magical arrow, fired it towards his glowing eye.\n")
    ib:AddMsg("True did it fly, right through his eye, on into his brain. Dead at last, I went to retrieve my special arrow. \n")
    ib:AddMsg("\n   As I yanked out the arrow, one last death spasm caused a claw, dripping in foul black ichor to scrape across my face, blinding me instantly.\n")
    ib:AddMsg("When I woke up again I was alive, thanks to the tabernacle, still blind, but with an ability to probe with my mind and to pass on to others my ^archery^ skill.\n")
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

local function topArchery()
    ib:SetTitle("Chereth's Archery Info")
    ib:SetMsg("\nYes, there are three archery skills:\n")
    ib:AddMsg("\nBow Archery is the most common firing arrows.\n")
    ib:AddMsg("\nSling Archery allows fast firing stones with less damage.\n")
    ib:AddMsg("\nCrossbow Archery uses x-bows and bolts. Slow but powerful.\n")
    ib:AddMsg("\nWell, there are the three different archery skills.\n\nI can teach you only °*ONE*° of them.\n")
    ib:AddMsg("You have to stay with it then. So choose wisely. \n")
    ib:AddMsg("But before I teach you I have a little ^quest^ for you.")
    ib:AddMsg("So, want to know what this ^quest^ is ?\n")
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
if q_mgr_1:GetStatus() < game.QSTAT_DONE then
    tl:AddTopics("startq1", topStartQ1) 
    tl:AddTopics("acceptq1", topAcceptQ1) 
    tl:AddTopics("checkq1", topCheckQ1) 
    tl:AddTopics("finishq1 #.*", topFinishQ1) 
end
tl:AddTopics("quest", topQuest)
tl:AddTopics("archery", topArchery)
tl:AddTopics("eyes", topEyes)

tl:CheckMessage(event)
