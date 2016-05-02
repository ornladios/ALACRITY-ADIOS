#ifndef ALACRITY_SERIALIZATION_LEGACY_H_
#define ALACRITY_SERIALIZATION_LEGACY_H_

#include "alacrity-types.h"
#include "alacrity-memstream.h"

ALError ALSerializeMetadataLegacy(const ALMetadata *metadata, memstream_t *ms);

uint64_t ALGetMetadataSizeLegacy(const ALMetadata *metadata);

ALError ALDeserializeMetadataLegacy(ALMetadata *metadata, memstream_t *ms);

#endif /*ALACRITY_SERIALIZATION_LEGACY_H_*/
