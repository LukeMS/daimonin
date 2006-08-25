-- Shows how to write movement behaviours

local ai = event.me:GetAI()

-- Find best friend (with a friendshiop of at least 100)
local bestest_friend = nil
local friendship = 200
for i,obj in pairs(ai:GetKnownMobs()) do
    local obj_friendship = ai:GetFriendship(obj)
    if obj_friendship > friendship then
        friendship = obj_friendship
        bestest_friend = obj
    end
end

-- (TODO: should check if we can see our dear friend first)

if bestest_friend ~= nil then
    local dist, dir, dx, dy = event.me:GetVector(bestest_friend)
    if dist then
        if dist > 1 then
            -- Move towards friend
            ai:MoveRespondObject(bestest_friend)
        else
            -- Stay still
            ai:MoveRespondDirection(0)
        end
    end
end
