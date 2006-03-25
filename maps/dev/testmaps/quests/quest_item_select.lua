-- template for a "item quest with selectable reward" script
require("topic_list")
require("quest_check")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local q_name_1  = "Dev Item Select Test Quest"
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
if q_stat_1 == game.QSTAT_NO then
ib:SetTitle("Item Select Test Quest")
ib:AddMsg("[intro] This script has a selectable reward. Means mutiple items where you has to select one. The tricky part is to allow the user select of the items in the GUI only when the quest is solved. Thats done by the ib:SelectOff() command.")
ib:AddLink("Start Item Select Test Quest", "startq1")
else
ib:SetTitle("Item Select Test Quest solved?")
ib:AddMsg("[pending] You has done the quest?")
ib:AddLink("Finish Item Select Test Quest", "checkq1")
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
ib:AddSelect("quest-select-reward-name1", "cape.101", "i am the stats/description line") 
ib:AddSelect("quest-select-reward-name2", "cloak_leather.101", "i am the stats/description line") 
ib:AddSelect("quest-select-reward-name2", "robe_lgrey.101", "i am the stats/description line") 
end
local function quest_body1()
ib:SetMsg("[WHY] Get the item this test quest needs.")
ib:SetDesc("[WHAT] Bring me a °Item Select Test Helm°.\nOpen the chest.", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
if q_stat_1 ~= game.QSTAT_NO then
topicDefault()
else
ib:SetTitle(q_name_1)
quest_body1()
quest_icons1()
ib:SelectOff()
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
ib:SelectOff()
q_obj_1 = pl:AddQuest(q_name_1, game.QUEST_ITEM, q_step_1, q_step_1, q_level_1, q_skill_1, ib:Build())
if q_obj_1 ~= null then
q_obj_1:AddQuestItem(1, "quest_object", "helm_leather.101", "Item Select Test Helm")
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
ib:SetTitle("FINAL CHECK: Item Select Test Quest")
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
local words = string.split(event.message)
q_obj_1:RemoveQuestItem()
q_obj_1:SetQuestStatus(-1)
q_stat_1 = game.QSTAT_DONE
pl:Sound(0, 0, 2, 0)
if words[2]=="#1" then  
pl:CreateObjectInsideEx("cape", 1,1)
elseif words[2]=="#2" then  
pl:CreateObjectInsideEx("cloak", 1,1)
else
pl:CreateObjectInsideEx("robe", 1,1)
end
pl:AddMoneyEx(1,2,0,0)
ib:SetTitle("QUEST END: Item Select Test Quest")
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
tl:AddTopics("finishq1 #.*", topFinishQ1) 
end
tl:CheckMessage(event)
