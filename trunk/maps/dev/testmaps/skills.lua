require("topic_list");

activator = event.activator
whoami = event.me

tl = TopicList()

tl:AddTopics("learn (.*)",
	function(skillname)
		skill = game:GetSkillNr(skillname)
		if skill == -1 then
			whoami:SayTo(activator,"Unknown skill \""..skillname.."\"" )
		else
			if activator:FindSkill(skill) ~= nil then
				whoami:SayTo(activator,"You already learned this skill." )
			else
				activator:AcquireSkill(skill, game.LEARN)
			end
		end
	end
)

tl:SetDefault("\nI am the Skillgiver.\nSay ^learn <skillname>^ or ^unlearn <skillname>^ (Note: unlearn skills is not implemented atm - the ideas of the daimonin skill system is to collect the skills - and don't lose them).")

tl:CheckMessage(event)
