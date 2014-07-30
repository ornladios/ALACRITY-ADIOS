#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <alacrity-part-manip.h>
#include <alacrity-types.h>
#include <ALUtil.h>

ALError ALEncoderConfigure(ALEncoderConfig  *config,
                           int              significantBits,
                           ALDatatype       datatype,
                           ALIndexForm      indexForm) {

	// Check parameters
	assert(significantBits > 0 && significantBits < 32); // 31 is a limitation of the current code
	assert(ALDatatypeIsDefined(datatype));
	assert(indexForm == ALCompressionIndex || indexForm == ALInvertedIndex || indexForm == ALCompressedInvertedIndex);

	config->significantBits = significantBits;
	config->elementSize = ALDatatypeGetSize(datatype);
	config->datatype = datatype;
	config->indexForm = indexForm;

	return ALErrorNone;
}
