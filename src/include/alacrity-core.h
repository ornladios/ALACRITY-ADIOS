#ifndef ALACRITY_CORE_H_
#define ALACRITY_CORE_H_

#include <alacrity-types.h>

/* Return description string of error */
char * ALErrorString(ALError e);

// Destroy an ALPartitionData structure produced by ALACRITY
ALError ALPartitionDataDestroy(ALPartitionData *data);

#endif