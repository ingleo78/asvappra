#ifndef __GAMIN_FAM_H__
#define __GAMIN_FAM_H__ 1

#ifdef __cplusplus
extern "C" {
#endif
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
typedef struct FAMConnection FAMConnection;
typedef FAMConnection *FAMConnectionPtr;
struct FAMConnection {
    int fd;
    void *client;
};
#define FAMCONNECTION_GETFD(fc) ((fc)->fd)
typedef struct FAMRequest FAMRequest;
typedef FAMRequest *FAMRequestPtr;
struct FAMRequest {
    int reqnum;
};
#define FAMREQUEST_GETREQNUM(fr) ((fr)->reqnum)
typedef enum FAMCodes {
    FAMChanged=1,
    FAMDeleted=2,
    FAMStartExecuting=3,
    FAMStopExecuting=4,
    FAMCreated=5,
    FAMMoved=6,
    FAMAcknowledge=7,
    FAMExists=8,
    FAMEndExist=9
} FAMCodes;
typedef struct  FAMEvent {
    FAMConnection* fc;
    FAMRequest fr;
    char *hostname;
    char filename[PATH_MAX];
    void *userdata;
    FAMCodes code;
} FAMEvent;
extern int FAMOpen(FAMConnection* fc);
extern int FAMOpen2(FAMConnection* fc, const char* appName);
extern int FAMClose(FAMConnection* fc);
extern int FAMMonitorDirectory(FAMConnection *fc, const char *filename, FAMRequest* fr, void* userData);
extern int FAMMonitorFile(FAMConnection *fc, const char *filename, FAMRequest* fr, void* userData);
extern int FAMMonitorDirectory2(FAMConnection *fc, const char *filename, FAMRequest* fr);
extern int FAMMonitorFile2(FAMConnection *fc, const char *filename, FAMRequest* fr);
extern int FAMMonitorCollection(FAMConnection *fc, const char *filename, FAMRequest* fr, void* userData, int depth, const char* mask);
extern int FAMSuspendMonitor(FAMConnection *fc, const FAMRequest *fr);
extern int FAMResumeMonitor(FAMConnection *fc, const FAMRequest *fr);
extern int FAMCancelMonitor(FAMConnection *fc, const FAMRequest *fr);
extern int FAMNextEvent(FAMConnection *fc, FAMEvent *fe);
extern int FAMPending(FAMConnection* fc);
extern int FAMErrno;
extern int FAMDebugLevel(FAMConnection *fc, int level);
#define FAM_DEBUG_OFF 0
#define FAM_DEBUG_ON  1
#define FAM_DEBUG_VERBOSE 2
extern const char *FamErrlist[];
extern int FAMNoExists(FAMConnection *fc);
#ifdef __cplusplus
}
#endif
#endif