dt = DataTable("data_store_test")

msg = dt:get("message")

if msg then
	if dt:get("activator") ~= nil then
		event.me:SayTo(event.activator, "\nA message from " .. dt:get("activator").name .. ":\n" .. msg)
	else
		event.me:SayTo(event.activator, "\nA message from an unkown object:\n" .. msg)
	end
else
	event.activator:Write("The lost soul remains quiet.", game.COLOR_RED)
end
