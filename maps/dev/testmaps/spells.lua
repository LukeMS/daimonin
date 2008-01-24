require("topic_list");
require("interface_builder")

local pl 		= event.activator
local me        = event.me
local msg       = string.lower(event.message)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    ib:SetTitle("Add Skills to Player")
    ib:AddMsg("I am the Spellgiver.\nSay ^learn <spellname>^ or ^unlearn <spellname>^")
    pl:Interface(1, ib:Build())
end

tl = TopicList()

-- Shows how to use captures in topics
tl:AddTopics("learn (.*)",
    function(spellname)
        spell = game:GetSpellNr(spellname)
        if spell == -1 then
            me:SayTo(pl,"Unknown spell \""..spellname.."\"" )
        else
            if pl:DoKnowSpell(spell) then
                me:SayTo(pl,"You already learned this spell." )
            else
                pl:AcquireSpell(spell, game.LEARN)
            end
        end
        topicDefault()
    end
)

tl:AddTopics("unlearn (.*)",
    function(spellname)
        spell = game:GetSpellNr(spellname)
        if spell == -1 then
            me:SayTo(pl,"Unknown spell \""..spellname.."\"" )
        else
            if not pl:DoKnowSpell(spell) then
                me:SayTo(pl,"You don't know this spell." )
            else
                pl:AcquireSpell(spell, game.UNLEARN)
            end
        end
        topicDefault()
    end
)

tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)

tl:CheckMessage(event)
