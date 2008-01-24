-- template for a "item quest" script
require("topic_list")
require("quest_check")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local q_name_1  = "Dev Item Test Quest"
local q_step_1  = 0
local q_level_1  = 1
local q_skill_1  = game.ITEM_SKILL_NO

local q_obj_1   = pl:GetQuest(q_name_1)
local q_stat_1  = Q_Status(pl, q_obj_1, q_step_1, q_level_1, q_skill_1)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
if q_stat_1 < game.QSTAT_DONE then
ib:AddMsg("[DEVMSG] The quest status is: ".. q_stat_1 .."\n\n")
ib:AddIcon(0,"booze_generic.101",0)
if q_stat_1 == game.QSTAT_NO then
ib:SetTitle("Item Test Quest")
ib:AddMsg("[INTRO] A quest which needs an item delivered. The item is inside the quest object inside the player which is given the player when this quest is accepted. There is a QUEST_TRIGGER inside a object (chest or mob) which will move a visible copy to the players inventory.")
ib:AddLink("Start Item Test Quest", "startq1")
else
ib:SetTitle("Item Test Quest solved?")
ib:AddMsg("[pending] You has done the quest?")
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
ib:SetDesc("[WHAT] Bring me a °Item Test Helm°.\nKill the ant or open the chest.", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
if q_stat_1 ~= game.QSTAT_NO then
topicDefault()
else
ib:SetTitle(q_name_1)
quest_body1()
quest_icons1()
ib:SetAccept(nil, "acceptq1") 
ib:SetDecline(nil, "hi") 
pl:Interface(1, ib:Build())
end
end

-- accepted: start the quest
local function topAcceptQ1()
if q_stat_1 == game.QSTAT_NO then
quest_body1()
quest_icons1()
q_obj_1 = pl:AddQuest(q_name_1, game.QUEST_ITEM, q_step_1, q_step_1, q_level_1, q_skill_1, ib:Build())
if q_obj_1 ~= null then
q_obj_1:AddQuestItem(1, "quest_object", "helm_leather.101", "Item Test Helm")
q_stat_1 = Q_Status(pl, q_obj_1, q_step_1, q_level_1, q_skill_1)
pl:Sound(0, 0, 2, 0)
pl:Write("You take the quest '"..q_name_1.."'.", game.COLOR_NAVY)
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
ib:SetTitle("FINAL CHECK: Item Test Quest")
ib:SetMsg("[DEVMSG] The quest status is: ".. q_stat_1 .."\n\n")
if q_stat_1 ~= game.QSTAT_SOLVED then
ib:AddMsg("[not-done-text] Come back if you have it!\n")
Q_List(q_obj_1, ib)
ib:SetButton("Back", "hi") 
else
ib:AddMsg("[final-text] Very well done! You found the helm.\n")
ib:SetDesc("here it is...", 1, 2, 0, 0)
quest_icons1()
Q_List(q_obj_1, ib)
ib:SetAccept(nil, "finishq1") 
ib:SetDecline(nil, "hi") 
end
pl:Interface(1, ib:Build())
end
end

-- done: finish quest and give reward
local function topFinishQ1()
if q_stat_1 ~= game.QSTAT_SOLVED then
topicDefault()
else
q_obj_1:RemoveQuestItem()
q_obj_1:SetQuestStatus(-1)
q_stat_1 = game.QSTAT_DONE
pl:Sound(0, 0, 2, 0)
pl:CreateObjectInsideEx("shield", 1,1)
pl:AddMoneyEx(1,2,0,0)
ib:SetTitle("QUEST END: Item Test Quest")
ib:SetMsg("Very well done! Here is your reward!")
ib:SetButton("Ok", "hi") 
pl:Interface(1, ib:Build())
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