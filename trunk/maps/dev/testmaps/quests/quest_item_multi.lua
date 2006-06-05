-- template for a "item quest" script3
require("topic_list")
require("quest_check")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local q_name_1  = "Dev Item Test Quest Multi 1"
local q_step_1  = 0
local q_level_1 = 1
local q_skill_1 = game.ITEM_SKILL_NO
local q_obj_1   = pl:GetQuest(q_name_1)
local q_stat_1  = Q_Status(pl, q_obj_1, q_step_1, q_level_1, q_skill_1)

local q_name_2  = "Dev Item Test Quest Multi 2"
local q_step_2  = 0
local q_level_2 = 1
local q_skill_2 = game.ITEM_SKILL_NO
local q_obj_2   = pl:GetQuest(q_name_2)
local q_stat_2  = Q_Status(pl, q_obj_2, q_step_2, q_level_2, q_skill_2, "Dev Item Test Quest Multi 1")

local q_name_3  = "Dev Item Test Quest Multi 3"
local q_step_3  = 0
local q_level_3 = 1
local q_skill_3 = game.ITEM_SKILL_NO
local q_obj_3   = pl:GetQuest(q_name_3)
local q_stat_3  = Q_Status(pl, q_obj_3, q_step_3, q_level_3, q_skill_3)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

-- thats the "start/main screen"
local function topicDefault()
	ib:SetTitle("I have Work for you!")
	ib:AddMsg("[intro] Because we can have more as one pending quest in this script at once, we need here a somewhat neutral intro text. The different quests are accessed by the links.")
	if q_stat_1 < game.QSTAT_DONE then
		ib:AddMsg("\n\n[DEVMSG] The multi quest-1 status is: ".. q_stat_1)
		if q_stat_1 == game.QSTAT_NO then
			ib:AddLink("Start Item Test Quest Multi 1", "startq1")
			ib:AddMsg("\nQuest 1 avaible\nStart me, finish me, then quest2 will be avaible.")
		else
			ib:AddLink("Finish Item Test Quest Multi 1", "checkq1")
			ib:AddMsg("\nYou are done with Quest 1?\nIts needed for quest2.")
		end
	end
	if q_stat_2 < game.QSTAT_DONE then
		ib:AddMsg("\n\n[DEVMSG] The multi quest-2 status is: ".. q_stat_2)
		if q_stat_2 == game.QSTAT_NO then
			ib:AddLink("Start Item Test Quest Multi 2", "startq2")
			ib:AddMsg("\nQuest 2 avaible\nYou have finished Quest 1!\Great, now i have this:\nStart me, finish me.")
		else
			ib:AddLink("Finish Item Test Quest Multi 2", "checkq2")
			ib:AddMsg("\nYou are done with Quest 2?")
		end
	end
	if q_stat_3 < game.QSTAT_DONE then
		ib:AddMsg("\n\n[DEVMSG] The multi quest-3 status is: ".. q_stat_3)
		if q_stat_3 == game.QSTAT_NO then
			ib:AddLink("Start Item Test Quest Multi 3", "startq3")
			ib:AddMsg("\nQuest 3 avaible\nStart me, finish me.")
		else
			ib:AddLink("Finish Item Test Quest Multi 3", "checkq3")
			ib:AddMsg("\nYou are done with Quest 3?")
		end
	end
	if q_stat_1 >= game.QSTAT_DONE and q_stat_2 >= game.QSTAT_DONE and q_stat_3 >= game.QSTAT_DONE then
		pl:Write(me.name .." has nothing to say.", game.COLOR_NAVY)
		pl:Interface(-1, "") 
		return
	end
	pl:Interface(1, ib:Build())
end

-- Quest 1 parts
-- quest body (added to player quest obj for quest list)
local function quest_icons1()
	ib:AddIcon("quest-reward-name", "shield_kite.101", "i am the stats/description line") 
end
local function quest_body1()
	ib:SetMsg("[WHY] Deliver the item for this multi quest example.")
	ib:SetDesc("[WHAT] Bring me a °Item Test Helm Multi 1°.\nOpen the chest #1.", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
	if q_stat_1 ~= game.QSTAT_NO then
		topicDefault()
		return
	end
	ib:SetTitle(q_name_1)
	quest_body1()
	quest_icons1()
	ib:SetAccept(nil, "acceptq1") 
	ib:SetDecline(nil, "hi") 
	pl:Interface(1, ib:Build())
end

-- accepted: start the quest
local function topAcceptQ1()
	if q_stat_1 == game.QSTAT_NO then
		quest_body1()
		quest_icons1()
		q_obj_1 = pl:AddQuest(q_name_1, game.QUEST_ITEM, q_step_1, q_step_1, q_level_1, q_skill_1, ib:Build())
		if q_obj_1 ~= null then
			q_obj_1:AddQuestItem(1, "quest_object", "helm_leather.101", "Item Test Helm Multi 1")
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
		return
	end
	ib:SetTitle("FINAL CHECK: Item Test Quest Multi 1")
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

-- done: finish quest and give reward
local function topFinishQ1()
	if q_stat_1 ~= game.QSTAT_SOLVED then
	topicDefault()
		return
	end
	q_obj_1:RemoveQuestItem()
	q_obj_1:SetQuestStatus(-1)
	q_stat_1 = game.QSTAT_DONE
	pl:Sound(0, 0, 2, 0)
	pl:CreateObjectInsideEx("shield_kite", 1,1)
	pl:AddMoneyEx(1,2,0,0)
	ib:SetTitle("QUEST END: Item Test Quest Multi 1")
	ib:SetMsg("Very well done! Here is your reward!")
	ib:SetButton("Ok", "hi") 
	pl:Interface(1, ib:Build())
end

-- Quest 2 parts
-- quest body (added to player quest obj for quest list)
local function quest_icons2()
	ib:AddIcon("quest-reward-name", "shield_eye.101", "i am the stats/description line") 
end
local function quest_body2()
	ib:SetMsg("[WHY] Deliver the item for this multi quest example.")
	ib:SetDesc(" [WHAT] Bring me a °Item Test Helm Multi 2°.\nOpen the chest #2", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ2()
	if q_stat_2 ~= game.QSTAT_NO then
		topicDefault()
		return
	end
	ib:SetTitle(q_name_2)
	quest_body2()
	quest_icons2()
	ib:SetAccept(nil, "acceptq2") 
	ib:SetDecline(nil, "hi") 
	pl:Interface(1, ib:Build())
end

-- accepted: start the quest
local function topAcceptQ2()
	if q_stat_2 == game.QSTAT_NO then
		quest_body2()
		quest_icons2()
		q_obj_2 = pl:AddQuest(q_name_2, game.QUEST_ITEM, q_step_2, q_step_2, q_level_2, q_skill_2, ib:Build())
		if q_obj_2 ~= null then
			q_obj_2:AddQuestItem(1, "quest_object", "helm_leather.101", "Item Test Helm Multi 2")
			q_stat_2 = Q_Status(pl, q_obj_2, q_step_2, q_level_2, q_skill_2, "Dev Item Test Quest Multi 1")
			pl:Sound(0, 0, 2, 0)
			pl:Write("You take the quest '"..q_name_2.."'.", game.COLOR_NAVY)
		end
		ib = InterfaceBuilder()
		ib:SetHeader(me, me.name)
	end
	topicDefault()
end

-- try to finish: check the quest
local function topCheckQ2()
	if q_stat_2 == game.QSTAT_NO then
		topicDefault()
		return
	end
	ib:SetTitle("FINAL CHECK: Item Test Quest Multi 2")
	ib:SetMsg("[DEVMSG] The quest status is: ".. q_stat_2 .."\n\n")
	if q_stat_2 ~= game.QSTAT_SOLVED then
		ib:AddMsg("[not-done-text] Come back if you have it!\n")
		Q_List(q_obj_2, ib)
		ib:SetButton("Back", "hi") 
	else
		ib:AddMsg("[final-text] Very well done! You found the helm.\n")
		ib:SetDesc("here it is...", 1, 2, 0, 0)
		quest_icons2()
		Q_List(q_obj_2, ib)
		ib:SetAccept(nil, "finishq2") 
		ib:SetDecline(nil, "hi") 
	end
	pl:Interface(1, ib:Build())
end

-- done: finish quest and give reward
local function topFinishQ2()
	if q_stat_2 ~= game.QSTAT_SOLVED then
		topicDefault()
		return
	end
	q_obj_2:RemoveQuestItem()
	q_obj_2:SetQuestStatus(-1)
	q_stat_2 = game.QSTAT_DONE
	pl:Sound(0, 0, 2, 0)
	pl:CreateObjectInsideEx("shield_eye", 1,1)
	pl:AddMoneyEx(1,2,0,0)
	ib:SetTitle("QUEST END: Item Test Quest Multi 2")
	ib:SetMsg("Very well done! Here is your reward!")
	ib:SetButton("Ok", "hi") 
	pl:Interface(1, ib:Build())
end

-- Quest 3 parts
-- quest body (added to player quest obj for quest list)
local function quest_icons3()
	ib:AddIcon("quest-reward-name", "shield_high.101", "i am the stats/description line") 
end
local function quest_body3()
	ib:SetMsg("[WHY] Deliver the item for this multi quest example.")
	ib:SetDesc("[WHAT] Bring me a °Item Test Helm Multi 3°.\nOpen the chest #3.", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ3()
	if q_stat_3 ~= game.QSTAT_NO then
	topicDefault()
		return
	end
	ib:SetTitle(q_name_1)
	quest_body3()
	quest_icons3()
	ib:SetAccept(nil, "acceptq3") 
	ib:SetDecline(nil, "hi") 
	pl:Interface(1, ib:Build())
end

-- accepted: start the quest
local function topAcceptQ3()
	if q_stat_3 == game.QSTAT_NO then
		quest_body3()
		quest_icons3()
		q_obj_3 = pl:AddQuest(q_name_3, game.QUEST_ITEM, q_step_3, q_step_3, q_level_3, q_skill_3, ib:Build())
		if q_obj_3 ~= null then
			q_obj_3:AddQuestItem(1, "quest_object", "helm_leather.101", "Item Test Helm Multi 3")
			q_stat_3 = Q_Status(pl, q_obj_3, q_step_3, q_level_3, q_skill_3)
			pl:Sound(0, 0, 2, 0)
			pl:Write("You take the quest '"..q_name_3.."'.", game.COLOR_NAVY)
		end
		ib = InterfaceBuilder()
		ib:SetHeader(me, me.name)
	end
	topicDefault()
end

-- try to finish: check the quest
local function topCheckQ3()
	if q_stat_3 == game.QSTAT_NO then
		topicDefault()
		return
	end
	ib:SetTitle("FINAL CHECK: Item Test Quest Multi 3")
	ib:SetMsg("[DEVMSG] The quest status is: ".. q_stat_3 .."\n\n")
	if q_stat_3 ~= game.QSTAT_SOLVED then
		ib:AddMsg("[not-done-text] Come back if you have it!\n")
		Q_List(q_obj_3, ib)
		ib:SetButton("Back", "hi") 
	else
		ib:AddMsg("[final-text] Very well done! You found the helm.\n")
		ib:SetDesc("here it is...", 1, 2, 0, 0)
		quest_icons3()
		Q_List(q_obj_3, ib)
		ib:SetAccept(nil, "finishq3") 
		ib:SetDecline(nil, "hi") 
	end
	pl:Interface(1, ib:Build())
end

-- done: finish quest and give reward
local function topFinishQ3()
	if q_stat_3 ~= game.QSTAT_SOLVED then
		topicDefault()
		return
	end
	q_obj_3:RemoveQuestItem()
	q_obj_3:SetQuestStatus(-1)
	q_stat_3 = game.QSTAT_DONE
	pl:Sound(0, 0, 2, 0)
	pl:CreateObjectInsideEx("shield", 1,1)
	pl:AddMoneyEx(1,2,0,0)
	ib:SetTitle("QUEST END: Item Test Quest Multi 3")
	ib:SetMsg("Very well done! Here is your reward!")
	ib:SetButton("Ok", "hi") 
	pl:Interface(1, ib:Build())
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
if q_stat_2 < game.QSTAT_DONE then
	tl:AddTopics("startq2", topStartQ2) 
	tl:AddTopics("acceptq2", topAcceptQ2) 
	tl:AddTopics("checkq2", topCheckQ2) 
	tl:AddTopics("finishq2", topFinishQ2) 
end
if q_stat_3 < game.QSTAT_DONE then
	tl:AddTopics("startq3", topStartQ3) 
	tl:AddTopics("acceptq3", topAcceptQ3) 
	tl:AddTopics("checkq3", topCheckQ3) 
	tl:AddTopics("finishq3", topFinishQ3) 
end
tl:CheckMessage(event)
