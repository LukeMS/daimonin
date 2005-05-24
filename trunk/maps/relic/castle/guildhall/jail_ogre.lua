require("topic_list")

activator = event.activator
me = event.me
quest_arch_name = "letter"
quest_item_name = "Frah'aks letter"

function topicDefault()
	activator:Write(me.name .. " listens to you without answer.", game.COLOR_WHITE)
end

function topicGreeting()
	qitem = activator:CheckQuestObject(quest_arch_name, quest_item_name)
	item = activator:CheckInventory(1, quest_arch_name, quest_item_name)
	skill = game:GetSkillNr("find traps")
	if qitem == nil and item ~= nil then
		me:Communicate("/grin " .. activator.name)
		me:SayTo(activator, [[

Ashahk! Yo bring me note!
Kobold chief bad time now, ha?
Now me will teach you!
Say ^teach me find traps^ now!]])

	else
		if qitem ~= nil then
			me:SayTo(activator, [[

Ashahk! Yo want me teaching yo more ^find traps^?
Will teach for money.]])

		else
			me:SayTo(activator, [[

Yo shut up.
Yo grack zhal hihzuk alshzu...
Me mighty ogre chief.
Me ^warrior^ will destroy yo. They come.
Guard and ^Kobolds^ will die then.]])

		end
	end
end

function topicStillThere()
	me:Communicate("/spit " .. activator.name)
end

function topicTeachMeFindTraps()
	skill = game:GetSkillNr("find traps")
	if skill == -1 then
		me:SayTo(activator, "Unknown skill - find traps.")
	else
		sobj = activator:GetSkill(game.TYPE_SKILL, skill)
		if sobj == None then
			qitem = activator:CheckQuestObject(quest_arch_name, quest_item_name)
			item = activator:CheckInventory(1, quest_arch_name, quest_item_name)
			if qitem == nil and item ~= nil then
				activator:AddQuestObject(quest_arch_name, quest_item_name)
				activator:Write(me.name .. " takes " .. item.name .. " from your inventory.", game.COLOR_WHITE)
				item:Remove()
                if skill ~= -1 and activator:FindSkill(skill) == nil then
				    me:SayTo(activator, "Here we go!")
				    me.map:Message(me.x, me.y, game.MAP_INFO_NORMAL, "Frah'ak teaches some ancient skill.", game.COLOR_YELLOW)
				    activator:AcquireSkill(skill, game.LEARN)
                else
                    me:SayTo(activator, "You already know skill find traps?!")
                end
			else
				me:SayTo(activator, "\nNah, bring Frah'ak note from ^kobolds^ first!")
			end
		else
			slevel = sobj.level + 1
			eobj = activator:GetSkill(game.TYPE_EXPERIENCE, game.EXP_AGILITY)
			if eobj == nil or eobj.level < slevel then
				me:SayTo(activator, "Ho, yo agility too low to teach!!")
			else
				amount = slevel * slevel * (50 + slevel) * 3
				if activator:PayAmount(amount) == 1 then
					activator:Write("You pay Frah'ak " .. activator:ShowCost(amount) .. ".", 0)
					me:SayTo(activator, "Here we go!")
					me.map:Message(me.x, me.y, game.MAP_INFO_NORMAL, "Frah'ak teaches some ancient skill.", game.COLOR_YELLOW)
					activator:SetSkill(game.TYPE_SKILL, skill, slevel, 0)
				else
					me:SayTo(activator, "Ho, yo not enough money to pay Frah'ak!!")
				end
			end
		end
	end
end

function topicFindTraps()
	skill = game:GetSkillNr("find traps")
	if skill == -1 then
		me:SayTo(activator, "Unknown skill - find traps.")
	else
		sobj = activator:GetSkill(game.TYPE_SKILL, skill)
		if sobj == nil then
			qitem = activator:CheckQuestObject(quest_arch_name, quest_item_name)
			item = activator:CheckInventory(1, quest_arch_name, quest_item_name)
			if qitem == nil and item ~= nil then
				me:SayTo(activator, "\nFrah'ak tell yo truth!\n Say ^teach me find traps^ now!")
			else
				me:SayTo(activator, "\nNah, bring Frah'ak note from ^kobolds^ first!")
			end
		else
			slevel = sobj.level + 1
			eobj = activator:GetSkill(game.TYPE_EXPERIENCE, game.EXP_AGILITY)
			if  eobj ~= nil and eobj.level >= slevel then
				me:SayTo(activator, "Find traps lvl " .. slevel .. " will cost you\n" .. activator:ShowCost(slevel * slevel * (50 + slevel) * 3) .. ".")
				me:SayTo(activator, "Say to me ^teach me find traps^ for teaching!!", 1)
			else
				me:SayTo(activator, "Ho, yo agility to low to teach!!")
			end
		end
	end
end

tl = TopicList()
tl:SetDefault(topicDefault)
tl:AddGreeting(nil, topicGreeting)
tl:AddTopics("still there, frah'ak??", topicStillThere)
tl:AddTopics("warrior", "Me big chief. Me ogre destroy you.\nStomp on. Dragon kakka.")
tl:AddTopics("kobolds", [[

Kobolds traitors!
Give gold for note, kobolds don't bring note to ogres.
Me tell you: Kill kobold chief!
Me will teach you ^find traps^ skill!
Show me note i will teach you.
Kobolds in hole next room. Secret entry in wall.]])

tl:AddTopics("teach me find traps", topicTeachMeFindTraps)
tl:AddTopics("find traps", topicFindTraps)

tl:CheckMessage(event)