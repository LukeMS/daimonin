#ifndef __P_UNICODE_H
#define __P_UNICODE_H

#define UNICODE_MAX_PATH_BUFFER 512
#define PATH_BACKBUFFER UNICODE_MAX_PATH_BUFFER * 2
#define APPDATA L"\\Application Data" /* It is safe */

extern void UTF_CopyExecutablePath(char * dst, size_t maxlen);
extern void UTF_FindUserDir(char * dst, size_t maxlen);
extern void GetDaimoninVersion(char * dst, size_t maxlen);
extern void GetDaimoninBase(char * dst, const char * separator, size_t maxlen);
extern void UTF_FindOutputPath(char * dst, size_t maxlen);
extern void UTF_Redirect_std();

#endif
