#ifndef ALACRITY_CORE_H_
#define ALACRITY_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <alacrity-types.h>

/* Return description string of error */
char * ALErrorString(ALError e);

// Destroy an ALPartitionData structure produced by ALACRITY
ALError ALPartitionDataDestroy(ALPartitionData *data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
