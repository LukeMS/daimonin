-- Handler for the global logout event
--
-- Its main purpose is to save player datastores to disk

local pl = event.activator

-- TODO: this would be a nice place to unload the datastore, but
-- it might still be referenced by some script, so we can't do that.

-- Grommit 11-May-2008: wrap with validity check. It's possible for a script
-- to lose the player (believe me, I've done it!), and for this to be called
-- with a nil activator. The server then goes into an infinite loop.

if game:IsValid(pl) then
    if pl.experience == 0 then
        -- Don't save data for unused characters.
        -- Also, we _must_ delete the data from memory, since the character
        -- name will be reused. We just hope no script is still referencing it.
        _data_store.forget_player(pl.name)
    else
        -- For safety we save real player's datastore when they
        -- log out. It would be nice to be able to free it here too.
        -- Maybe we can move it over to a weak table?
        _data_store.save_player(pl.name, true)
    end
end

