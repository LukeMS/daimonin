event.me:Say("I'm a trigger script with value " .. event.me.weight_limit)
if(event.me.slaying == 'override') then
    event.returnvalue = 1
end
