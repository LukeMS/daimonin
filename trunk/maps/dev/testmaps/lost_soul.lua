local b_msg = false

ds = DataStore("data_store_test")
msg = ds:Get("message")

if msg then
	ac = ds:Get("activator")
	b_msg = true
	if ac ~= nil then
		event.me:SayTo(event.activator, "\nA global message from " .. ac.name .. ":\n" .. msg)
	else
		event.me:SayTo(event.activator, "\nA global message from an unkown object:\n" .. msg)
	end
end

ds = DataStore("data_store_test", event.activator)
msg = ds:Get("message")

if msg then
	ac = ds:Get("activator")
	b_msg = true
	if ac ~= nil then
		event.me:SayTo(event.activator, "\nA local message from " .. ac.name .. " for you:\n" .. msg)
	else
		event.me:SayTo(event.activator, "\nA local message from an unkown object for you:\n" .. msg)
	end
end

if not b_msg then
	event.activator:Write("The lost soul remains quiet.", game.COLOR_RED)
end
