/*	=========================================================================
 *	=                        	       					=
 *	= --------------------------------------------------------------------- =
 *	=  						                        =
 *	=  c) 2001/2002  by Michael Toennies					=
 *	=========================================================================
 *
 *	Modified	 by 			reason
 *	========	 == 			======
 *	23.06.00	 Michael Toennies  	Init module.
 */

#if !defined(__WRAPPER_H)
#define __WRAPPER_H

/* include here the hardware depend headers */
#ifdef __WIN_32
#include "win32.h"
#elif __LINUX
#include <cflinux.h>
#define _malloc(__d,__s) malloc(__d)
#endif

#define MAX_METASTRING_BUFFER 128*2013

typedef enum _LOGLEVEL
{
        LOG_MSG,
        LOG_ERROR,
        LOG_DEBUG
} _LOGLEVEL;
#define LOGLEVEL LOG_DEBUG
extern void LOG (int logLevel, char *format, ...);

extern char * GetCacheDirectory(void);
extern char * GetBitmapDirectory(void);
extern char * GetSfxDirectory(void);
extern char * GetMediaDirectory(void);
extern char * GetIconDirectory(void);

extern Boolean SYSTEM_Start(void);
extern Boolean SYSTEM_End(void);
int attempt_fullscreen_toggle(SDL_Surface **surface, uint32 *flags);
uint32 get_video_flags(void);
void parse_metaserver_data(char *info);

#endif
