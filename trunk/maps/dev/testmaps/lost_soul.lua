local b_msg = false

dt = DataTable("data_store_test")
msg = dt:get("message")

if msg then
	ac = dt:get("activator")
	b_msg = true
	if ac ~= nil then
		event.me:SayTo(event.activator, "\nA global message from " .. ac.name .. ":\n" .. msg)
	else
		event.me:SayTo(event.activator, "\nA global message from an unkown object:\n" .. msg)
	end
end

dt = DataTable("data_store_test", event.activator)
msg = dt:get("message")

if msg then
	ac = dt:get("activator")
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
