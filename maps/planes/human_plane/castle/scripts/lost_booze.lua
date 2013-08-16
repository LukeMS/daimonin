-- template for a "item quest" script
require("topic_list")
require("quest_builder")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)
local qb        = QuestBuilder()

local q_name_1  = "Lost Booze"
local q_step_1  = 0
local q_level_1  = 4
local q_skill_1  = game.ITEM_SKILL_NO

local ib = InterfaceBuilder()

local function questGoal(questnr)
    qb:AddQuestItem(questnr, 1, "quest_object", "booze.101", "Boozie")
    pl:Sound(0, 0, 2, 0)
    pl:Write("You take the quest '".. qb:GetName(questnr) .."'.", game.COLOR_NAVY)
end

qb:AddQuest("Lost Booze", game.QUEST_ITEM, q_level_1, q_skill_1, nil, nil, 1, questGoal, nil)
local questnr = qb:Build(pl)
local q_stat_1 = qb:GetStatus(1)

local function topicDefault()
	if q_stat_1 < game.QSTAT_DONE then
		if q_stat_1 == game.QSTAT_NO then
			ib:SetTitle("Lost Booze")
			ib:AddMsg("'Lo der chap. Yer look like uh nice feller, so take yeh a set an' 'ave a bet o meh boozish... Jus' uh sec... whas tis? Someun tooked meh boozie! Why dat varmet, I swears, wen I git em--ah, oh. I mustuv lef meh boozun a 'ome in meh booze ches'. Would yeh be uh nice feller an' fetch meh boozit fo' some coin?")
			ib:AddLink("Yessir", "startq1")
		else
			ib:SetTitle("Found Meh Boozie?")
			ib:AddMsg("Yeh found meh boozie yet? I been awaitin 'ere fo' it.")
			ib:AddLink("Yup, here it is", "checkq1")
		end
	else
		pl:Write(me.name .." has nothing to say.", game.COLOR_NAVY)
		pl:Interface(game.GUI_NPC_MODE_NO) 
		return
	end
	pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_icons1()
	ib:AddIcon("Silver", "silvercoin.101", "A Silver Coin") 
end
local function quest_body1()
	ib:SetMsg("Bring meh boozie from meh 'ouse fer meh. Meh 'ouse is aroun' on the east part o' town.")
	ib:SetDesc("Rememer, brin' meh ~Boozie~ from meh booze ches'.", 0, 4, 0, 0)
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
		pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
	end
end

-- accepted: start the quest
local function topAcceptQ1()
	if q_stat_1 == game.QSTAT_NO then
		qb:RegisterQuest(questnr, me, ib)
		pl:Interface(game.GUI_NPC_MODE_NO)
	else
		topicDefault()
	end
end

-- try to finish: check the quest
local function topCheckQ1()
if q_stat_1 == game.QSTAT_NO then
topicDefault()
else
ib:SetTitle("Yous gots meh boozie?!")
if q_stat_1 ~= game.QSTAT_SOLVED then
ib:AddMsg("Meh Boozie? Yous gots meh boozie? Giver 'ere! Wha? No, you don' gots meh boozie... comere when yeh gots it.\n")
ib:SetButton("Back", "hi") 
else
ib:AddMsg("Meh boozie! Teres meh boozun... Tankee much.\n")
ib:SetDesc("'Ere, ave yer coin.", 0, 4, 0, 0)
ib:SetAccept(nil, "finishq1") 
ib:SetDecline(nil, "hi") 
end
pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end
end

-- done: finish quest and give reward
local function topFinishQ1()
	if q_stat_1 ~= game.QSTAT_SOLVED then
		topicDefault()
	else
		pl:Sound(0, 0, 2, 0)
		pl:AddMoneyEx(0,4,0,0)
		ib:SetTitle("Lost Booze")
		ib:SetMsg("Yeh ave yerself uh day o goodun!")
		qb:Finish(questnr)
		ib:SetButton("Ok", "hi") 
		pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
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