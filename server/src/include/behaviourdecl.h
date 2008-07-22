/* The format of this file is quite straight-forward, except for the use of
 * commas. Make sure you get the commas right, or you will get nasty error
 * messages (sometimes "Macro used with too many args") when compiling.
 * Any syntax errors in this file will lead to virtually undecipherable error
 * messages from the compiler or preprocessor, but such is life ;-)
 *
 * TODO: One of the ideas with this file is that it should be parsed
 * by the editor to generate a nice UI for behaviour configuration.
 */
/* Oh, and if someone really wants to get advanced:
 * TODO: Make the list syntax more obvious (add comma separation and
 *       consistent list termination)
 * TODO: get rid of the need to name the behaviour for each parameter
 */

/*
 * Declaration syntax (in sortof EBNF form):
 *
 * <classlist> := <behaviourclass> | <behaviourclass> <classlist>
 * <behaviourclass> :=
 *      "BehaviourClass(" classname "," <behaviourlist> ")"
 *
 * <behaviourlist> := "NIL" | <behaviour>+
 * <behaviour> :=
 *      "Behaviour(" behaviourname "," behaviourfunc "," <opt_parameterlist> ")"
 *
 * <parameterlist> := "NIL" | <parameter>+
 * <parameter> :=
 *      "Parameter(" behaviourname "," parametername "," type "," flags "," defaultvalue ")"
 *
 * - NIL is used for empty lists and not for terminating lists.
 *   Lists are (currently) not comma-separated.
 * - Behaviour.behaviourfunc and Parameter.behaviourname are not important
 *   for mapmakers and are only relevant for the internal handling of processes.
 */

/**
 * Processes are the use of senses and internal thought processes.
 * They don't take any game-time to run, so all of a mobs processes
 * are always executed
 */
BehaviourClass(PROCESSES,
    /**Classify all detected mobs and assign friendship values to them.
     * (The parameters here modifies initial friendship value at first sight)
     * Should always come after look_for_other_mobs */
    Behaviour(FRIENDSHIP, ai_friendship,
        /** Creatures of the same alignment (currently: friendly/non-friendly) */
        Parameter(FRIENDSHIP, SAME_ALIGNMENT, INTEGER, OPTIONAL, 100)
        /** Creatures of the opposite alignment (currently: friendly/non-friendly).
         * Set this to 0 to have mobs that only attack if attacked first. */
        Parameter(FRIENDSHIP, OPPOSITE_ALIGNMENT, INTEGER, OPTIONAL, -100)
        /** Attitude against a race */
        Parameter(FRIENDSHIP, RACE, STRINGINT, MULTI | OPTIONAL, "X:0")
        /** Attitude against an archetype */
        Parameter(FRIENDSHIP, ARCH, STRINGINT, MULTI | OPTIONAL, "X:0")
        /** Attitude against a named mob/player */
        Parameter(FRIENDSHIP, NAME, STRINGINT, MULTI | OPTIONAL, "X:0")
        /** Attitude against any player */
        Parameter(FRIENDSHIP, PLAYER, INTEGER, OPTIONAL, 0)
        /** Attitude against a named AI group, see the "groups" behaviour */
        Parameter(FRIENDSHIP, GROUP, STRINGINT, MULTI | OPTIONAL, "X:0")
        /** Attitude against the followers of a named god */
        Parameter(FRIENDSHIP, GOD, STRINGINT, MULTI | OPTIONAL, "X:0")
    )

    /** Determines the initial attraction of a mob towards another object or mob.
     * (The parameters here modifies initial attraction value at first sight)
     * Should always come after look_for_other_mobs and
     * look_for_objects. */
    Behaviour(ATTRACTION, ai_attraction,
        /** Attraction towards an archetype */
        Parameter(ATTRACTION, ARCH, STRINGINT, MULTI | OPTIONAL, "X:0")
        /** Attraction towards a named object/mob/player */
        Parameter(ATTRACTION, NAME, STRINGINT, MULTI | OPTIONAL, "X:0")
        /** Attraction towards an object type */
        Parameter(ATTRACTION, TYPE, TYPEINT, MULTI | OPTIONAL, "X:0")
        /** Attraction towards cursed objects */
        Parameter(ATTRACTION, CURSED, INTEGER, OPTIONAL, 0)
        /** Attraction towards cursed objects of a specific type */
        Parameter(ATTRACTION, CURSEDTYPE, TYPEINT, MULTI | OPTIONAL, "X:0")
        /** Attraction towards any player */
        Parameter(ATTRACTION, PLAYER, INTEGER, OPTIONAL, 0)
    )

    /** Configures the group membership of the mob, not a real behaviour.
     * This can be used to set up one or more groups for the attitude
     * behaviour's "group" parameter to match against.
     */
    Behaviour(GROUPS, ai_fake_process,
        /** Membership in one or more named AI groups */
        Parameter(GROUPS, NAME, STRING, MULTI | MANDATORY, NULL)
    )

    /** Simply look around for other mobs nearby */
    Behaviour(LOOK_FOR_OTHER_MOBS, ai_look_for_other_mobs, NIL)

    /** Look around for interesting objects nearby
     * (see ATTRACTION for the definition of interesting) */
    Behaviour(LOOK_FOR_OBJECTS, ai_look_for_objects, NIL)

    /** Choose the known mob with the lowest negative friendship as the current enemy
     * should always come after friendship */
    Behaviour(CHOOSE_ENEMY, ai_choose_enemy,
            /** The anti-luring protection works by ignoring any
             * enemy at a specific distance from the "home" point.
             * In most cases "home" is the starting point of the mob,
             * but for mobs following wayponts it is the last recently
             * treaded square on a path towards the current waypoint.
             * For pets "home" is the current position of the owner.
             * Deactivate by using a distance setting of -1.
             */
            Parameter(CHOOSE_ENEMY, ANTILURE_DISTANCE, INTEGER, OPTIONAL, -1)
    )

    /** Plugin interface for processes */
    Behaviour(PLUGIN_PROCESS, ai_plugin_process,
        /** Plugin name, e.g. "lua" */
        Parameter(PLUGIN_PROCESS, PLUGIN, STRING, MANDATORY, NULL)
        /** Behaviour name visible to plugin */
        Parameter(PLUGIN_PROCESS, BEHAVIOUR, STRING, MANDATORY, NULL)
        /** Behaviour parameters visible to plugin */
        Parameter(PLUGIN_PROCESS, OPTIONS, STRING, OPTIONAL, "")
    )
)

/**
 * Moves are the basic movement behaviours. Basically a move can
 * decide to take a step in a direction, stand still or not decide anything.
 * If a move decides something no other move will execute, otherwise the
 * decision is handed over to the next move behaviour in the list.
 *
 * Moves that never passes the decision on are called "terminal moves", and
 * should be the last ones in a mobs moves list.
 */
BehaviourClass(MOVES,
    /** Always stands still. Terminal */
    Behaviour(STAND_STILL, ai_stand_still, NIL)

    /** Stand still if the mob's sleep flag is set. Should normally be
     * high up in the moves list (except maybe for sleepwalkers and mobs
     * that never sleep and never can be forced to sleep). */
    Behaviour(SLEEP, ai_sleep, NIL)

    /** Move around erratically by taking steps in random directions. Terminal. */
    Behaviour(MOVE_RANDOMLY, ai_move_randomly,
        /** Limits the x-distance from the starting point */
        Parameter(MOVE_RANDOMLY, XLIMIT, INTEGER, OPTIONAL, -1)
        /** Limits the y-distance from the starting point */
        Parameter(MOVE_RANDOMLY, YLIMIT, INTEGER, OPTIONAL, -1)
    )

    /** Makes sure the mob never stands 100% still */
    Behaviour(DONT_STAND_STILL, ai_dont_stand_still,
        /** Max allowed still time (number of moves) */
        Parameter(DONT_STAND_STILL, MAX_IDLE_TIME, INTEGER, OPTIONAL, 1)
    )

    /** Move towards the mob's starting point. Terminal. */
    Behaviour(MOVE_TOWARDS_HOME, ai_move_towards_home, NIL)

    /** Move towards the current enemy if any */
    Behaviour(MOVE_TOWARDS_ENEMY, ai_move_towards_enemy, NIL)

    /** Move towards the current enemy's last known position if the current position is unknown. */
    Behaviour(MOVE_TOWARDS_ENEMY_LAST_KNOWN_POS, ai_move_towards_enemy_last_known_pos, NIL)

    /** Search around for an enemy that can't be seen (until the memory of the enemy times out). */
    Behaviour(SEARCH_FOR_LOST_ENEMY, ai_search_for_lost_enemy, NIL)

    /** Move towards the current active waypoint if any */
    Behaviour(MOVE_TOWARDS_WAYPOINT, ai_move_towards_waypoint, NIL)

    /** Avoid stepping on repulsive items. Hint: put early in list
     * for high strictness (won't cross repulsive items even to attack)
     * or late for more relaxed avoidance. */
    Behaviour(AVOID_REPULSIVE_ITEMS, ai_avoid_repulsive_items, NIL)

    /** Move towards a nearby attractive item, investigate it and then continue */
    Behaviour(INVESTIGATE_ATTRACTION, ai_investigate_attraction,
        /** Investigate an archetype */
//        Parameter(INVESTIGATE_ATTRACTION, ARCH, STRING, MULTI | OPTIONAL, NULL)
        /** Investigate a named object/mob/player */
//        Parameter(INVESTIGATE_ATTRACTION, NAME, STRING, MULTI | OPTIONAL, NULL)
    NIL)

    /** Runs away from the current enemy if scared. */
    Behaviour(RUN_AWAY_FROM_ENEMY, ai_run_away_from_enemy,
        /** Become scared if current_hp < HP_THRESHOLD % of max_hp */
        Parameter(RUN_AWAY_FROM_ENEMY, HP_THRESHOLD, INTEGER, OPTIONAL, 10)
    )
    
    /** Runs away from the most repulsive object if scared. */
    Behaviour(RUN_AWAY_FROM_REPULSIVE_OBJECT, ai_run_away_from_repulsive_object,
        /** Become scared if at least this close to the object */
        Parameter(RUN_AWAY_FROM_REPULSIVE_OBJECT, DISTANCE_THRESHOLD, INTEGER, OPTIONAL, 5)
        /** Become scared if repulsion is at least this low */
        Parameter(RUN_AWAY_FROM_REPULSIVE_OBJECT, REPULSION_THRESHOLD, INTEGER, OPTIONAL, -30)
    )

    /** Try to stay at a certain distance from the enemy,
     * good for mobs with distance attacks */
    Behaviour(KEEP_DISTANCE_TO_ENEMY, ai_keep_distance_to_enemy,
        Parameter(KEEP_DISTANCE_TO_ENEMY, MIN_DIST, INTEGER, OPTIONAL, 4)
        Parameter(KEEP_DISTANCE_TO_ENEMY, MAX_DIST, INTEGER, OPTIONAL, 6)
    )

    /** To be removed, or renamed to "hit and run" */
    Behaviour(STEP_BACK_AFTER_SWING, ai_step_back_after_swing,
        Parameter(STEP_BACK_AFTER_SWING, DIST, INTEGER, OPTIONAL, 3)
    )

    /** Try to stay out of the line of fire as much as possible
     * if we belive our enemy uses distance attacks */
    Behaviour(AVOID_LINE_OF_FIRE, ai_avoid_line_of_fire, NIL)

    /** Try to move to a position with free line of fire towards
     * enemy. A good archer's answer to AVOID_LINE_OF_FIRE */
    Behaviour(OPTIMIZE_LINE_OF_FIRE, ai_optimize_line_of_fire, NIL)

    /** Do not allow moves that takes us further away from home. Use
     * early in the moves list. */
    Behaviour(STAY_NEAR_HOME, ai_stay_near_home,
            /** Maximum allowed distance from home. */
            Parameter(STAY_NEAR_HOME, MAX_DIST, INTEGER, OPTIONAL, 12)
            /** Enables true eucilidian distance (default is diagonal distance). 
             * Uses more CPU resources.  */
            Parameter(STAY_NEAR_HOME, EUCLIDIAN_DISTANCE, INTEGER, OPTIONAL, 0)
    )

    /** Plugin interface for moves */
    Behaviour(PLUGIN_MOVE, ai_plugin_move,
        /** Plugin name, e.g. "lua" */
        Parameter(PLUGIN_MOVE, PLUGIN, STRING, MANDATORY, NULL)
        /** Behaviour name visible to plugin */
        Parameter(PLUGIN_MOVE, BEHAVIOUR, STRING, MANDATORY, NULL)
        /** Behaviour parameters visible to plugin */
        Parameter(PLUGIN_MOVE, OPTIONS, STRING, OPTIONAL, "")
    )
)

/** Actions are misc actions that takes time, but aren't movement.
 * The handling of this class isn't 100% decided on yet, so be prepared
 * for changes in the future.
 * Currently we go though the list of actions and execute the first that
 * wants to be executed, ignoring the rest.
 */
BehaviourClass(ACTIONS,
    /** Attack the current enemy with melee weapons if within range */
    Behaviour(MELEE_ATTACK_ENEMY, ai_melee_attack_enemy, NIL)

    /** Attack the current enemy with bow or throw weapons if within range
     * and line of fire */
    Behaviour(BOW_ATTACK_ENEMY, ai_bow_attack_enemy, NIL)

    /** Attack the current enemy with distance spells if within range
     * and line of fire */
    Behaviour(SPELL_ATTACK_ENEMY, ai_spell_attack_enemy, NIL)

    /** Cast healing/restoring spells on friends. Casting on self is
     * always implicit, but casting on others will only be done if
     * their friendship is above the specified values AND a value is
     * specified. */
    Behaviour(HEAL_FRIEND, ai_heal_friend,
        /** Friendship threshold for healing */
        Parameter(HEAL_FRIEND, HEALING_MIN_FRIENDSHIP, INTEGER, OPTIONAL, 0)
        /** Friendship threshold for cure poison */
        Parameter(HEAL_FRIEND, CURE_POISON_MIN_FRIENDSHIP, INTEGER, OPTIONAL, 0)
        /** Friendship threshold for cure disease */
        Parameter(HEAL_FRIEND, CURE_DISEASE_MIN_FRIENDSHIP, INTEGER, OPTIONAL, 0)
    )

    /** Plugin interface for actions */
    Behaviour(PLUGIN_ACTION, ai_plugin_action,
        /** Plugin name, e.g. "lua" */
        Parameter(PLUGIN_ACTION, PLUGIN, STRING, MANDATORY, NULL)
        /** Behaviour name visible to plugin */
        Parameter(PLUGIN_ACTION, BEHAVIOUR, STRING, MANDATORY, NULL)
        /** Behaviour parameters visible to plugin */
        Parameter(PLUGIN_ACTION, OPTIONS, STRING, OPTIONAL, "")
    )
)
