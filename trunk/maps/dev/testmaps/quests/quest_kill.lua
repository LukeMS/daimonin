-- template for a "kill quest" script
require("topic_list")
require("quest_check")
require("interface_builder")

local activator = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local q_name_1  = "kill test quest"
local q_step_1  = 0
local q_obj_1   = activator:CheckQuest(q_name_1)
local q_stat_1  = QKill_Status(q_obj_1, q_step_1)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
ib:SetTitle("MAIN: Kill Test Quest")
ib:SetMessage("[intro] The quest status is: ".. q_stat_1)
if q_stat_1 == game.QSTAT_NO then
ib:AddToMessage("\n\nTo solve this quest:\nKill °1 red ant° (normal or big ones) and °2 kobolds°.\n\n Return when done.")
ib:AddLink("Start Kill Test Quest", "startq1")
elseif q_stat_1 ~= game.QSTAT_DONE then
ib:AddToMessage("\n\n[pending quest] Kill Test Quest not done\n")
QKill_List(q_obj_1, ib)
ib:AddLink("Finish Kill Test Quest", "checkq1")
else
ib:AddToMessage("\n\n[quest] Kill Test Quest - done.")
end
activator:Interface(1, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_icons()
ib:AddIcon("quest-reward-name", "shield.101", "i am the stats/description line") 
end
local function quest_body()
ib:SetMessage("[Explain what & why]\nKill °1 red ant° and °2 kobolds°.")
ib:SetReward("Reward", "[reward-text] This is the reward you will get", 1, 2, 0, 0)
quest_icons()
end

-- start: accept or decline the quest
local function topStartQ1()
if q_stat_1 ~= game.QSTAT_NO then
topicDefault()
else
ib:SetTitle("START: Kill Test Quest")
ib:SetMessage("[start] The quest status is: ".. q_stat_1 .."\n\n")
quest_body()
ib:SetAccept(nil, "acceptq1") 
ib:SetDecline(nil, "hi") 
activator:Interface(1, ib:Build())
end
end

-- accepted: start the quest
local function topAcceptQ1()
if q_stat_1 == game.QSTAT_NO then
q_obj_1 = activator:AddQuest(q_name_1, game.QUEST_KILL, q_step_1, "")

-- add the targets
q_obj_1:AddKillQuestTarget(1, "ant_red", "red ant", "big red ant" )
q_obj_1:AddKillQuestTarget(2, "kobold", "kobold")

q_stat_1 = QKill_Status(q_obj_1, q_step_1)
activator:Sound(0, 0, 2, 0)
quest_body()
activator:Write("You take the quest 'Kill Test Quest'.", game.COLOR_NAVY)
ib = InterfaceBuilder()
ib:SetHeader(me, me.name)
end
topicDefault()
end

-- try to finish: check the quest
local function topCheckQ1()
if q_stat_1 == game.QSTAT_DONE or q_stat_1 == game.QSTAT_NO then
topicDefault()
else
ib:SetTitle("FINAL CHECK: Kill Test Quest")
ib:SetMessage("[final check] The quest status is: ".. q_stat_1 .."\n")
if q_stat_1 ~= game.QSTAT_SOLVED then
QKill_List(q_obj_1, ib)
ib:AddToMessage("\nCome back if you have it!")
ib:SetButton("Back", "hi") 
else
ib:SetReward("Reward", "[reward-final-text] Very well done - You killed all.\nhere it is...", 1, 2, 0, 0)
quest_icons()
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
q_stat_1 = QKill_Status(q_obj_1, q_step_1)
activator:Sound(0, 0, 2, 0)
activator:CreateObjectInsideEx("shield", 1,1)
activator:AddMoneyEx(1,2,0,0)
ib:SetTitle("QUEST END: Kill Test Quest")
ib:SetMessage("Very well done! Here is your reward!")
ib:SetButton("Back", "hi") 
activator:Interface(1, ib:Build())
end
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("startq1", topStartQ1) 
tl:AddTopics("acceptq1", topAcceptQ1) 
tl:AddTopics("checkq1", topCheckQ1) 
tl:AddTopics("finishq1", topFinishQ1) 
tl:CheckMessage(event)
