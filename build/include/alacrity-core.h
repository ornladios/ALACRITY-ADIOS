#pragma once

#include <alacrity-types.h>

/* Return description string of error */
char * ALErrorString(ALError e);

// Destroy an ALPartitionData structure produced by ALACRITY
ALError ALPartitionDataDestroy(ALPartitionData *data);

