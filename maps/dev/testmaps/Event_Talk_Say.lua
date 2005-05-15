etype = event.options
if etype == nil then
    etype = "UNKNOWN"
end

event.me:SayTo(event.activator,"This is a " .. etype .. " script!")
event.me:SayTo(event.activator,"You said " .. event.message)
