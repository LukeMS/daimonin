require("topic_list");

activator = event.activator
whoami = event.me

tl = TopicList()

tl:AddTopics('detect curse',
    function()
        if activator:PayAmount(200) then
            whoami:SayTo(activator,"\nOk, i will cast a 'detect curse' for 2s on you.")
            activator:Write("You pay the money.", 0)
            whoami:CastSpell(activator,game:GetSpellNr("detect curse"), 1,0,"")
        else
            whoami:SayTo(activator,"\nSorry, you do not have enough money.")
        end
    end
)

tl:AddTopics('remove curse',
    function()
        if activator:PayAmount(300) then
            whoami:SayTo(activator,"\nOk, i will cast a 'remove curse' for 3s on you.")
            activator:Write("You pay the money.", 0)
            whoami:CastSpell(activator,game:GetSpellNr("remove curse"), 1,0,"")
        else
            whoami:SayTo(activator,"\nSorry, you do not have enough money.")
        end
    end
)

tl:AddTopics('remove damnation',
    function()
        if activator:PayAmount(200) then
            whoami:SayTo(activator,"\nOk, i will cast a 'remove damnation' for 30s on you.")
            activator:Write("You pay the money.", 0)
            whoami:CastSpell(activator,game:GetSpellNr("remove damnation"), 1,0,"")
        else
            whoami:SayTo(activator,"\nSorry, you do not have enough money.")
        end
    end
)

tl:AddTopics('detect magic',
    function()
        if activator:PayAmount(200) then
            whoami:SayTo(activator,"\nOk, i will cast a 'detect magic' for 2s on you.")
            activator:Write("You pay the money.", 0)
            whoami:CastSpell(activator,game:GetSpellNr("detect magic"), 1,0,"")
        else
            whoami:SayTo(activator,"\nSorry, you do not have enough money.")
        end
    end
)

tl:AddTopics('remove depletion',
    function()
        if activator:PayAmount(200) then
            whoami:SayTo(activator,"\nOk, i will cast a 'remove depletion' for 35s on you.")
            activator:Write("You pay the money.", 0)
            whoami:CastSpell(activator,game:GetSpellNr("remove depletion"), 1,0,"")
        else
            whoami:SayTo(activator,"\nSorry, you do not have enough money.")
        end
    end
)

tl:AddTopics('identify',
    function()
        object = activator:FindMarkedObject()
        if object == nil then
            whoami:SayTo(activator,"\nYou must mark the item first")
        else
            if activator:PayAmount(2000) then
                whoami:SayTo(activator,"\nOk, i will cast a 'identify' for 20s over the "..object.name..".")
                activator:Write("You pay the money.", 0)
                whoami:IdentifyItem(activator, object, game.IDENTIFY_MARKED)
            else
                whoami:SayTo(activator,"\nSorry, you do not have enough money.")
            end
        end
    end
)

tl:AddTopics('identify all',
    function()
        if activator:PayAmount(5000) then
            whoami:SayTo(activator,"\nOk, i will cast a 'identify all' for 50s.")
            activator:Write("You pay the money.", 0)
            whoami:IdentifyItem(activator, None, game.IDENTIFY_ALL)
        else
            whoami:SayTo(activator,"\nSorry, you do not have enough money.")
        end
    end
)

tl:AddTopics('food', '\nYour stomach is filled again.',
function() activator.food = 999 end)

tl:SetDefault([[

Welcome to my shop. We have what you want!
As service I can cast 'detect curse' or 'detect magic' for 20s on your items. I can also 'identify' single items for only 40s or all for 2g.
Say ^detect curse^ or ^magic^, ^remove curse^ or ^damnation^ or ^depletion^, ^identify^ or ^identify all^ if you need my service.")
]])

tl:CheckMessage(event)
