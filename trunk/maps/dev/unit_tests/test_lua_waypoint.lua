-- Test case for bug #0000536: using .map index on object without a map crashes server

me = event.me
activator = event.activator
activator.title = "before"
local foo = me.map  -- accessing me.map, shouldn't crash server
activator.title = "checkpoint" -- this should be the title after the script is run
activator.title = foo.orig_path -- accessing nil.orig_path: should abort script.
activator.title = "after"
