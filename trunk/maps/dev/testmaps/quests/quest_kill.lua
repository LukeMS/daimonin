-- template for a "kill quest" script
require("topic_list")
require("quest_check")
require("interface_builder")

local activator = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local q_name_1  = "Dev Kill Test Quest"
local q_step_1  = 0
local q_level_1  = 1
local q_skill_1  = game.ITEM_SKILL_NO

local q_obj_1   = activator:GetQuest(q_name_1)
local q_stat_1  = Q_Status(activator, q_obj_1, q_step_1, q_level_1, q_skill_1)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
if q_stat_1 < game.QSTAT_DONE then
ib:AddToMessage("[DEVMSG] The quest status is: ".. q_stat_1 .."\n\n")
if q_stat_1 == game.QSTAT_NO then
ib:SetTitle("Kill Test Quest")
ib:AddToMessage("[intro] I need your help with...\nKill 1 Red Ant and 2 Kobolds.")
ib:AddLink("Start Kill Test Quest", "startq1")
else
ib:SetTitle("Kill Test Quest solved?")
ib:AddToMessage("[pending] You has done the quest?")
ib:AddLink("Finish Kill Test Quest", "checkq1")
end
else
activator:Write(me.name .." has nothing to say.", game.COLOR_NAVY)
activator:Interface(-1, "") 
return
end
activator:Interface(1, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_icons1()
ib:AddIcon("quest-reward-name", "shield.101", "i am the stats/description line") 
end
local function quest_body1()
ib:SetMessage("[Explain what & why]\nKill 1 Red Ant and 2 Kobolds.")
ib:SetReward("Reward", "[reward-text] This is the reward you will get", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
if q_stat_1 ~= game.QSTAT_NO then
topicDefault()
else
ib:SetTitle("START: Kill Test Quest")
quest_body1()
quest_icons1()
ib:SetAccept(nil, "acceptq1") 
ib:SetDecline(nil, "hi") 
activator:Interface(1, ib:Build())
end
end

-- accepted: start the quest
local function topAcceptQ1()
if q_stat_1 == game.QSTAT_NO then
quest_body1()
quest_icons1()
q_obj_1 = activator:AddQuest(q_name_1, game.QUEST_KILL, q_step_1, q_step_1, q_level_1, q_skill_1, ib:Build())
if q_obj_1 ~= null then

-- add the targets: chance (0=always), nrof to kill, mob definition
q_obj_1:AddQuestTarget(0, 1, "ant_red", "Red Ant", "Big Red Ant" )
q_obj_1:AddQuestTarget(0, 2, "kobold", "Kobold")
 
q_stat_1 = Q_Status(activator, q_obj_1, q_step_1, q_level_1, q_skill_1)
activator:Sound(0, 0, 2, 0)
activator:Write("You take the quest 'Kill Test Quest'.", game.COLOR_NAVY)
end
ib = InterfaceBuilder()
ib:SetHeader(me, me.name)
end
topicDefault()
end

-- try to finish: check the quest
local function topCheckQ1()
if q_stat_1 == game.QSTAT_NO then
topicDefault()
else
ib:SetTitle("FINAL CHECK: Kill Test Quest")
ib:SetMessage("[DEVMSG] The quest status is: ".. q_stat_1 .."\n\n")
if q_stat_1 ~= game.QSTAT_SOLVED then
ib:AddToMessage("[not-done-text] Come back if you have killed all!\n")
Q_List(q_obj_1, ib)
ib:SetButton("Back", "hi") 
else
ib:AddToMessage("[final-text] Very well done! All is done.\n")
ib:SetReward("Reward", "here it is...", 1, 2, 0, 0)
quest_icons1()
Q_List(q_obj_1, ib)
ib:SetAccept(nil, "finishq1") 
ib:SetDecline(nil, "hi") 
end
activator:Interface(1, ib:Build())
end
end

-- done: finish quest and give reward
local function topFinishQ1()
if q_stat_1 ~= game.QSTAT_SOLVED then
topicDefault()
else
q_obj_1:SetQuestStatus(-1)
q_stat_1 = game.QSTAT_DONE
activator:Sound(0, 0, 2, 0)
activator:CreateObjectInsideEx("shield", 1,1)
activator:AddMoneyEx(1,2,0,0)
ib:SetTitle("QUEST END: Kill Test Quest")
ib:SetMessage("Very well done! Here is your reward!")
ib:SetButton("Ok", "hi") 
activator:Interface(1, ib:Build())
end
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
if q_stat_1 < game.QSTAT_DONE then
tl:AddTopics("startq1", topStartQ1) 
tl:AddTopics("acceptq1", topAcceptQ1) 
tl:AddTopics("checkq1", topCheckQ1) 
tl:AddTopics("finishq1", topFinishQ1) 
end
tl:CheckMessage(event)
