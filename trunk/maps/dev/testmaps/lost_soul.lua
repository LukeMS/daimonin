dt = DataTable("data_store_test")

msg = dt:get("message")

if msg then
	ac = dt:get("activator")
	if ac ~= nil then
		event.me:SayTo(event.activator, "\nA message from " .. ac.name .. ":\n" .. msg)
	else
		event.me:SayTo(event.activator, "\nA message from an unkown object:\n" .. msg)
	end
else
	event.activator:Write("The lost soul remains quiet.", game.COLOR_RED)
end
