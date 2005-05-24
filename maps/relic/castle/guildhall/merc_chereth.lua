require("topic_list")

activator = event.activator
me = event.me
quest_arch_name = "head_ant_queen"
quest_item_name = "water well ant queen head"

qitem = activator:CheckQuestObject(quest_arch_name, quest_item_name)
item = activator:CheckInventory(1, quest_arch_name, quest_item_name)

function topicDefault()
	activator:Write(me.name .. " listens to you without answer.", game.COLOR_WHITE)
end

function topicGreeting()
	if qitem == nil then
		if item == nil then
			me:SayTo(activator, [[

Hello, mercenary. I am Supply Chief Chereth.
Fomerly Archery Commander Chereth,
before I lost my eyes.
Well, I still know alot about ^archery^.
Perhaps you want to ^learn^ an archery skill?]])

		else
			me:SayTo(activator, [[

The head! You have done it!
Now we can repair the water well.
Say ^teach^ to me now to learn an archery skill!]])

		end
	else
		me:SayTo(activator, [[

Hello ]] .. activator.name .. [[.
Good to see you back.
I have no quest for you or your ^archery^ skill.]])

	end
end

function topicLearn()
	if qitem ~= nil then
		me:SayTo(activator, "\nSorry, I can only teach you *one* archery skill.")
	else
		me:SayTo(activator, [[

Well, there are three different ^archery^ skills.
I can teach you only *ONE* of them.
You have to stay with it then. So choose wisely.
I can tell you more about ^archery^. But before i teach you i have a little ^quest^ for you.]])

	end
end

function topicTeachMeBow()
	if qitem ~= nil or item == nil then
		me:SayTo(activator, "\nI can't ^teach^ you this now.")
	else
		activator:AddQuestObject(quest_arch_name, quest_item_name)
		item:Remove()
		me:SayTo(activator, "Here we go!")
		me.map:Message(me.x, me.y, game.MAP_INFO_NORMAL, "Chereth teaches some ancient skill.", game.COLOR_YELLOW)
		activator:AcquireSkill(game:GetSkillNr("bow archery"), game.LEARN)
		activator:CreateObjectInside("bow_short", 1, 1)
		activator:Write("Chereth gives you a short bow.", game.COLOR_WHITE)
		activator:CreateObjectInside("arrow", 1, 12)
		activator:Write("Chereth gives you 12 arrows.", game.COLOR_WHITE)
	end
end

function topicTeachMeSling()
	if qitem ~= nil or item == nil then
		me:SayTo(activator, "\nI can't ^teach^ you this now.")
	else
		activator:AddQuestObject(quest_arch_name, quest_item_name)
		item:Remove()
		me:SayTo(activator, "Here we go!")
		me.map:Message(me.x, me.y, game.MAP_INFO_NORMAL, "Chereth teaches some ancient skill.", game.COLOR_YELLOW)
		activator:AcquireSkill(game:GetSkillNr("sling archery"), game.LEARN)
		activator:CreateObjectInside("sling_small", 1, 1)
		activator:Write("Chereth gives you a small sling.", game.COLOR_WHITE)
		activator:CreateObjectInside("sstone", 1, 12)
		activator:Write("Chereth gives you 12 sling stones.", game.COLOR_WHITE)
	end
end

function topicTeachMeCrossbow()
	if qitem ~= nil or item == nil then
		me:SayTo(activator, "\nI can't ^teach^ you this now.")
	else
		activator:AddQuestObject(quest_arch_name, quest_item_name)
		item:Remove()
		me:SayTo(activator, "Here we go!")
		me.map:Message(me.x, me.y, game.MAP_INFO_NORMAL, "Chereth teaches some ancient skill.", game.COLOR_YELLOW)
		activator:AcquireSkill(game:GetSkillNr("crossbow archery"), game.LEARN)
		activator:CreateObjectInside("crossbow_small", 1, 1)
		activator:Write("Chereth gives you a small crossbow.", game.COLOR_WHITE)
		activator:CreateObjectInside("bolt", 1, 12)
		activator:Write("Chereth gives you 12 bolts.", game.COLOR_WHITE)
	end
end

function topicQuest()
	if qitem ~= nil then
		me:SayTo(activator, "\nI have no quest for you after you helped us out.")
	else
		if item == nil then
			me:SayTo(activator, [[

Yes, we need your help first.
As supply chief the water support of this outpost
is under my command. We noticed last few days problems
with our main water source.
It seems a traveling hive of giant ants has invaded the
caverns under our water well.
Enter the well next to this house and kill the ant queen!
Bring me her head as a trophy and I will ^teach^ you.]])

		else
			me:SayTo(activator, [[

The head! You have done it!
Now we can repair the water well.
Say ^teach^ to me now to learn an archery skill!]])

		end
	end
end

function topicTeach()
	if qitem ~= nil then
		me:SayTo(activator, "\nSorry, I can only teach you *one* archery skill.")
	else
		if item == nil then
			me:SayTo(activator, [[

Where is the queen's head? I don't see it.
Solve the ^quest^ first and kill the ant queen.
Then I will teach you.]])

		else
			me:SayTo(activator, [[

As reward I will teach you an archery skill.
Choose wisely. I can only teach you *one* of three skills!!
You want some info about the ^archery^ skills?
If you know your choice tell me ^teach me bow^,
^teach me sling^ or ^teach me crossbow^.]])

		end
	end
end

local tl = TopicList()
tl:AddGreeting(nil, topicGreeting)
tl:AddTopics("archery", [[

Yes, there are three archery skills then
Bow Archery is the most common firing arrows.
Sling Archery allows fast firing stones with less damage.
Crossbow Archery uses x-bows and bolts. Slow but powerful.]])

tl:AddTopics("learn", topicLearn)
tl:AddTopics("teach me bow", topicTeachMeBow)
tl:AddTopics("teach me sling", topicTeachMeSling)
tl:AddTopics("teach me crossbow", topicTeachMeCrossbow)
tl:AddTopics("quest", topicQuest)
tl:AddTopics("teach", topicTeach)

tl:CheckMessage(event)