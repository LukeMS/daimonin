require("topic_list");

activator = event.activator
whoami = event.me 

tl = TopicList()

tl:addTopics("learn (.*)", 
	function(spellname)
		spell = game.GetSpellNr(spellname)
		if spell == -1 then
			whoami:SayTo(activator,"Unknown spell \""..spellname.."\"" )
		else
			if activator:DoKnowSpell(spell) then
				whoami:SayTo(activator,"You already learned this spell." )
			else
				activator:AcquireSpell(spell, game.LEARN)
			end
		end
	end
)

tl:addTopics("unlearn (.*)", 
	function(spellname)
		spell = game.GetSpellNr(spellname)
		if spell == -1 then
			whoami:SayTo(activator,"Unknown spell \""..spellname.."\"" )
		else
			if not activator:DoKnowSpell(spell) then
				whoami:SayTo(activator,"You don't know this spell." )
			else
				activator:AcquireSpell(spell, game.UNLEARN)
			end
		end
	end
)

tl:setDefault(
	"\nI am the Spellgiver.\nSay ^learn <spellname>^ or ^unlearn <spellname>^");

tl:checkMessage()
