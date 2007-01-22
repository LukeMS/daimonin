local me = event.me
local map = me.map
local apple = game:LocateBeacon("test_apple")
local void = event.other

me.name="init"

assert(game:IsValid(me), "I'm invalid")
assert(game:IsValid(apple), "Apple is invalid")
assert(game:IsValid(map), "My map is invalid")
assert(game:IsValid(void), "Void container is invalid")

me.title="preconditions passed"

coroutine.yield(0) -- first very simple yield
me.title="yield passed"

apple:Remove()
coroutine.yield(0) -- The garbage collection should now automatically invalidate the apple

assert(not game:IsValid(apple), "Apple is valid")
assert(not game:IsValid(map), "My map is valid") -- This is due to the temporary fix of map validity. The map reference shouldn't really become invalid.
assert(game:IsValid(me), "I'm invalid")
assert(game:IsValid(void), "Void container is invalid")

me.title="test1 passed"

coroutine.yield(0) -- During this yield the server should delete the map

if game:IsValid(map) then
    -- This may crash the server if the map was freed but not marked invalid
    print ("map.name="..map.name)
end
assert(not game:IsValid(map), "My map is valid")

coroutine.yield(0) -- It takes some extra time to invalidate "me"
assert(not game:IsValid(me), "I'm valid")
assert(game:IsValid(void), "Void container is invalid")

void.title="passed"
