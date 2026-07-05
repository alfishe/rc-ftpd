/* Compatibility wrapper for inetd.h
** Prevents DaemonMessage redefinition between inetd.h and bsdsocket.h
**
** The NDK has two versions of DaemonMessage:
** - inetd.h: uses dm_Id (lowercase d)
** - bsdsocket.h: uses dm_ID (uppercase D)
**
** Our code uses dm_ID, so we use bsdsocket.h's definition.
** We define INETD_H AFTER including bsdsocket.h so it defines DaemonMessage,
** then the original inetd.h is skipped when included later.
*/
#ifndef COMPAT_INETD_H
#define COMPAT_INETD_H

/* Include bsdsocket.h first - it will define DaemonMessage with dm_ID */
#include <libraries/bsdsocket.h>

/* Now define INETD_H to prevent the system inetd.h from being included
** (which would try to redefine DaemonMessage with dm_Id) */
#define INETD_H

/* Provide the other definitions from inetd.h that aren't in bsdsocket.h */
#ifndef DAEMONPORTNAME
#define DAEMONPORTNAME "inetd.ipc"
#endif

#ifndef _DAEMONPORT_DEFINED
#define _DAEMONPORT_DEFINED
struct DaemonPort {
  struct MsgPort dp_Port;
  __stdargs void (*dp_ExitCode)();
};
#endif

#ifndef DMTYPE_UNKNOWN
#define DMTYPE_UNKNOWN   -1
#define DMTYPE_INTERNAL  0
#define DMTYPE_STREAM    SOCK_STREAM
#define DMTYPE_DGRAM     SOCK_DGRAM
#define DMTYPE_RAW       SOCK_RAW
#define DMTYPE_RDM       SOCK_RDM
#define DMTYPE_SEQPACKET SOCK_SEQPACKET
#endif

#ifndef DERR_LIB
#define DERR_LIB    0xA0
#define DERR_OBTAIN 0xA1
#endif

#endif /* COMPAT_INETD_H */
