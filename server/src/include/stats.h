#ifndef STATS_H
#define STATS_H

#define STATS_EVENT

typedef enum stats_event_type
{
    STATS_EVENT_FIRST,          /* Only used to indicate start of list */
    STATS_EVENT_STARTUP,        /* Use this event to log server starts */
    STATS_EVENT_SHUTDOWN,       /* Use this event to log server shutdowns */
    STATS_EVENT_PLAYER_DEATH,   /* player dies, format: char *name */
    STATS_EVENT_PVP_DEATH,      /* player dies in pvp: char *name, char *killername */
    STATS_EVENT_MESSAGE,        /* Use this to log any message string, format: char *msg */
    STATS_EVENT_ROLLBACK,       /* Use this message to inform db of a backup restore */
    STATS_EVENT_LAST            /* Only used to indicate end of list */
} stats_event_type;

#endif
