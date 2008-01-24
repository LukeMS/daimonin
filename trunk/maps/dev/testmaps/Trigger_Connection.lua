lever_beacon = game:LocateBeacon("lever_beacon")
lever = lever_beacon.environment
event.me:SayTo(event.activator, "Triggering voice controlled lever")
event.me:Apply(lever, game.APPLY_TOGGLE)