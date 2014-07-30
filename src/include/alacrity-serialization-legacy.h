#pragma once

#include <alacrity-types.h>

ALError ALSerializeMetadataLegacy(const ALMetadata *metadata, memstream_t *ms);

uint64_t ALGetMetadataSizeLegacy(const ALMetadata *metadata);

ALError ALDeserializeMetadataLegacy(ALMetadata *metadata, memstream_t *ms);
