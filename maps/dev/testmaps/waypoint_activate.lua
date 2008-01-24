require("topic_list");

activator = event.activator
whoami    = event.me
tl        = TopicList()

-- Test to trigger waypoints from scripts

tl:AddTopics("go", 
    function()
        tmp = whoami:CheckInventory(0, nil, "waypoint1")
        if not tmp then
            whoami:SayTo(activator, 'Oops... I seem to be lost.')
        else
            tmp.f_cursed = 1 -- this activates the waypoint
        end
    end
)

tl:SetDefault('Tell me to ^go^ to activate my waypoint')

tl:CheckMessage(event)
