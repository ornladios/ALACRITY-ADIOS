#include <alacrity-types.h>
#include <alacrity-core.h>
#include <ALUtil.h>

ALError ALPartitionDataDestroy(ALPartitionData *part) {
	if (part->ownsBuffers) {
		FREE(part->metadata.binLayout.binValues);
		FREE(part->metadata.binLayout.binStartOffsets);
		FREE(part->data);
		FREE(part->index);
	}
}
