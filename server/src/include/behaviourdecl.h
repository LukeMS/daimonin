/* The format of this file is quite straight-forward, except for the use of 
 * commas. Make sure you get the commas right, or you will get nasty error
 * messages (sometimes "Macro used with too many args") when compiling.
 * Any syntax errors in this file will lead to virtually undecipherable error
 * messages from the compiler or preprocessor, but such is life ;-)
 */
/*
 * TODO: documentation strings of behaviours and parameters
 *       (in a format easily parsed by the editor, and ignored by the
 *       server)
 */
/* Oh, and if someone really wants to get advanced:
 * TODO: Make the list syntax more obvious
 * TODO: get rid of the need to name the behaviour for each parameter
 */

/**
 * Processes are the use of senses and internal thought processes. 
 * They don't take any game-time to run, so all of a mobs processes
 * are always executed 
 */
BehaviourClass( PROCESSES, 
    /** Simply look around for other mobs nearby */
    Behaviour( LOOK_FOR_OTHER_MOBS, ai_look_for_other_mobs, NIL )
    /** Classify all detected mobs and assign friendship values to them.
     * Should always come after look_for_other_mobs */
    Behaviour( FRIENDSHIP, ai_friendship, NIL )
    /** Choose the known mob with the lowest negative friendship as the current enemy 
     * should always come after friendship */
    Behaviour( CHOOSE_ENEMY, ai_choose_enemy, NIL )
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
BehaviourClass( MOVES, 
    /** Always stands still. Terminal */
    Behaviour( STAND_STILL, ai_stand_still, NIL )
    /** Stand still if the mob's sleep flag is set. Should normally be
     * high up in the moves list (except maybe for sleepwalkers and mobs
     * that never sleep and never can be forced to sleep). */
    Behaviour( SLEEP, ai_sleep, NIL )
    /** Move around erratically by taking steps in random directions. Terminal. */
    Behaviour( MOVE_RANDOMLY, ai_move_randomly, 
        /** Limits the x-distance from the starting point */
        Parameter(MOVE_RANDOMLY, XLIMIT, INTEGER, OPTIONAL, -1)
        /** Limits the y-distance from the starting point */
        Parameter(MOVE_RANDOMLY, YLIMIT, INTEGER, OPTIONAL, -1)
    )
    /** Move towards the mob's starting point. Terminal. */
    Behaviour( MOVE_TOWARDS_HOME, ai_move_towards_home, NIL )
    /** Move towards the current enemy if any */
    Behaviour( MOVE_TOWARDS_ENEMY, ai_move_towards_enemy, NIL )
    /** Move towards the current enemy's last known position if the current position is unknown. */
    Behaviour( MOVE_TOWARDS_ENEMY_LAST_KNOWN_POS, ai_move_towards_enemy_last_known_pos, NIL )
    /** Search around for an enemy that can't be seen (until the memory of the enemy times out). */
    Behaviour( SEARCH_FOR_LOST_ENEMY, ai_search_for_lost_enemy, NIL )
    /** Move towards the current active waypoint if any */
    Behaviour( MOVE_TOWARDS_WAYPOINT, ai_move_towards_waypoint, NIL )
    /** Runs away from the current enemy if scared. */
    Behaviour( RUN_AWAY_FROM_ENEMY, ai_run_away_from_enemy, 
        /** Become scared if current_hp < HP_THRESHOLD % of max_hp */
        Parameter(RUN_AWAY_FROM_ENEMY, HP_THRESHOLD, INTEGER, OPTIONAL, 10)
     )
) 

/** TBD */
BehaviourClass( REACTION_MOVES, NIL )

/** Actions are misc actions that takes time, but aren't movement.
 * The handling of this class isn't 100% decided on yet, so be prepared
 * for changes in the future.
 * Currently we go though the list of actions and execute the first that
 * wants to be executed, ignoring the rest.
 */
BehaviourClass( ACTIONS, 
    /** Attack the current enemy with melee weapons if within range */
    Behaviour( MELEE_ATTACK_ENEMY, ai_melee_attack_enemy, NIL )       
    /** Attack the current enemy with bow or throw weapons if within range
     * and line of fire */
    Behaviour( BOW_ATTACK_ENEMY, ai_bow_attack_enemy, NIL )       
    /** Attack the current enemy with distance spells if within range
     * and line of fire */
    Behaviour( SPELL_ATTACK_ENEMY, ai_spell_attack_enemy, NIL )       
)
