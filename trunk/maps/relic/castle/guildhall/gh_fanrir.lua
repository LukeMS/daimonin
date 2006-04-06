-- Fanrir Quest - Guildhall Startquest 1
require("topic_list")
require("quest_check")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local q_name_1  = "Examine Fanrir's Sack"
local q_step_1  = 0
local q_end_1  = 1
local q_level_1  = 1
local q_skill_1  = game.ITEM_SKILL_NO

local q_obj_1   = pl:GetQuest(q_name_1)
local q_stat_1  = Q_Status(pl, q_obj_1, q_step_1, q_level_1, q_skill_1)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
if q_stat_1 < game.QSTAT_DONE then
if q_stat_1 == game.QSTAT_NO then
ib:SetTitle("Welcome to Daimonin!")
ib:AddMsg("I am Fanrir, the Guildhall Advisor.\n\nThis °Talk Interface° allows you to communicate with town people like me. You see the ~green line~ under this text? Thats called a LINK LINE. It will lead you to a QUEST or an EVENT!\n\nClick on it with your mouse OR use the TAB key until the text is shown in purple in the bottom line and press then RETURN.")
ib:AddLink("How to use the Talk Interface (TAB or mouse click)", "startq1")
else
ib:SetTitle("Normal Test Quest solved?")
ib:AddMsg("[pending] You has done the quest?")
ib:AddLink("Finish Normal Test Quest", "checkq1")
end
else
pl:Write(me.name .." has nothing to say.", game.COLOR_NAVY)
pl:Interface(-1, "") 
return
end
pl:Interface(1, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_body1()
ib:SetMsg("You has to learn how to use a container!\nA container is a chest, sack or bag. But also a shelf on a wall, a bookcase or a desk.\n\nNow, how you open a container?\n\na.) move over the container you want open.\nb.) use the °cursor keys° to move the blue scare over it\nc.) press the '°A°' key\n\nThat will open the container and show the items inside with a red border on the buttom. You can examine the stuff by moving the blue square with the cursor keys over it and pressing the '°E°' key.")
ib:SetDesc("This is your first °Quest°! Open my sack next to me and examine the cape inside. Then '°T°'alk again to me.", 0, 0, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
if q_stat_1 ~= game.QSTAT_NO then
topicDefault()
else
ib:SetTitle(q_name_1)
quest_body1()
ib:SetAccept(nil, "acceptq1") 
ib:SetDecline(nil, "hi") 
pl:Interface(1, ib:Build())
end
end

-- accepted: start the quest
local function topAcceptQ1()
if q_stat_1 == game.QSTAT_NO then
quest_body1()
q_obj_1 = pl:AddQuest(q_name_1, game.QUEST_NORMAL, q_step_1, q_end_1, q_level_1, q_skill_1, ib:Build())
if q_obj_1 ~= null then
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
ib:SetTitle("FINAL CHECK: Normal Test Quest")
ib:SetMsg("[DEVMSG] The quest status is: ".. q_stat_1 .."\n\n")
if q_stat_1 ~= game.QSTAT_SOLVED then
ib:AddMsg("[not-done-text] You has not looked in the chest?!\n")
Q_List(q_obj_1, ib)
ib:SetButton("Back", "hi") 
else
ib:AddMsg("[final-text] Very well done! You opened the chest!\n")
ib:SetDesc("here it is...", 0, 0, 0, 0)
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
q_obj_1:SetQuestStatus(-1)
q_stat_1 = game.QSTAT_DONE
pl:Sound(0, 0, 2, 0)
ib:SetTitle("You got it!")
ib:SetMsg("Very well done! I see you has understand it.\nNow you are ready for a real quest.")
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
