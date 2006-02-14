-- template for a "(base) item quest with selectable multi reward" script
require("topic_list")
require("interface_builder")

local activator = event.activator
local me = event.me
local msg = string.lower(event.message)

local q_name_1 = "test quest1"
local q_step_1 = 0

local q_obj_1 = activator:CheckQuest(q_name_1)
local q_item_1a = nil

-- local: check status for quest_1
local function QStatus1()
if q_obj_1 == nil or q_obj_1.magic < q_step_1 then
return game.QSTAT_NO
elseif q_obj_1.last_eat == -1 then
return game.QSTAT_DONE
elseif q_obj_1.magic == q_step_1 then
return game.QSTAT_ACTIVE
else
q_item_1a = activator:CheckInventory(1, "quest_object", "test quest1 item")
if q_item_1a == nil then
return game.QSTAT_INCOMPLETE
end
end
return game.QSTAT_SOLVED
end 

local q_stat_1 = QStatus1()

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
ib:SetTitle("MAIN: Test Item Quest")
ib:SetMessage("[intro] The quest status is: ".. q_stat_1)
if q_stat_1 == game.QSTAT_NO then
ib:AddToMessage("\n\nTo solve this quest:\nKill the °ant° OR open the °chest°\nBring back the °item° you will find.\ndone.")
ib:AddLink("Start Test Item Quest", "startq1")
elseif q_stat_1 ~= game.QSTAT_DONE then
ib:AddToMessage("\n\n[pending quest] Item Test Quest - where is my... and so on.")
ib:AddLink("Finish Test Item Quest", "checkq1")
else
ib:AddToMessage("\n\n[quest] Test Item Quest - done.")
end
activator:Interface(1, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_icons()
ib:AddIcon("quest-reward-name", "shield.101", "i am the stats/description line") 
ib:AddSelect("quest-select-reward-name1", "cape.101", "i am the stats/description line") 
ib:AddSelect("quest-select-reward-name2", "cloak_leather.101", "i am the stats/description line") 
ib:AddSelect("quest-select-reward-name2", "robe_lgrey.101", "i am the stats/description line") 
end
local function quest_body()
ib:SetMessage("[Explain what & why]\nkill the ant OR open the chest - bring the item you will find back")
ib:SetReward("Reward", "[reward-text] This is the reward you will get", 1, 2, 0, 0)
quest_icons()
end

-- start: accept or decline the quest
local function topStartQ1()
if q_stat_1 ~= game.QSTAT_NO then
topicDefault()
else
ib:SetTitle("START: Test Item Quest")
ib:SetMessage("[start] The quest status is: ".. q_stat_1 .."\n\n")
quest_body()
ib:SelectOff()
ib:SetAccept(nil, "acceptq1") 
ib:SetDecline(nil, "hi") 
activator:Interface(1, ib:Build())
end
end

-- accepted: start the quest
local function topAcceptQ1()
if q_stat_1 == game.QSTAT_NO then
activator:Sound(0, 0, 2, 0)
quest_body()
ib:SelectOff()
q_obj_1 = activator:AddQuest(q_name_1, game.QUEST_NORMAL, q_step_1, ib:Build())
q_stat_1 = QStatus1()
activator:Write("You take the quest 'Test Item Quest'.", game.COLOR_NAVY)
ib = InterfaceBuilder()
ib:SetHeader(me, me.name)
topicDefault()
else
topicDefault()
end
end

-- try to finish: check the quest
local function topCheckQ1()
if q_stat_1 == game.QSTAT_DONE or q_stat_1 == game.QSTAT_NO then
topicDefault()
else
if q_stat_1 ~= game.QSTAT_SOLVED then
ib:SetTitle("CHECK: Test Item Quest")
ib:SetMessage("[final check] The quest status is: ".. q_stat_1 .."\n\n")
ib:AddToMessage("Where is what i told you to bring me?\nCome back if you have it!")
ib:SetButton("Back", "hi") 
else
ib:SetTitle("FINAL CHECK: Test Item Quest")
ib:SetMessage("[final check] The quest status is: ".. q_stat_1 .."\n\n")
ib:SetReward("Reward", "[reward-final-text] Very well done - here it is...", 1, 2, 0, 0)
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
local words = string.split(event.message)
q_obj_1:SetQuestStatus(-1)
q_item_1a:DecreaseNrOf()
q_stat_1 = QStatus1()
activator:Sound(0, 0, 2, 0)
activator:CreateObjectInsideEx("shield", 1,1)
activator:AddMoneyEx(1,2,0,0)
if words[2]=="#1" then  
activator:CreateObjectInsideEx("cape", 1,1)
elseif words[2]=="#2" then  
activator:CreateObjectInsideEx("cloak", 1,1)
else
activator:CreateObjectInsideEx("robe", 1,1)
end
ib:SetTitle("QUEST END: Test Normal Quest")
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
tl:AddTopics("finishq1 #.*", topFinishQ1) 
tl:CheckMessage(event)
