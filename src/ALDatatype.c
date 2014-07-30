/*
 * ALDatatype.c
 *
 *  Created on: Nov 19, 2012
 *      Author: David A. Boyuka II
 */

#include <stdint.h>
#include <assert.h>

#include <alacrity-datatype.h>

_Bool ALDatatypeIsValid(ALDatatype type) {
	return type > MIN_DATATYPE_ENUM && type < MAX_DATATYPE_ENUM;
}

_Bool ALDatatypeIsDefined(ALDatatype type) {
	return ALDatatypeIsValid(type) && type != DATATYPE_UNDEFINED;
}

const char * ALDatatypeGetName(ALDatatype type) {
	switch (type) {
	case DATATYPE_FLOAT32: return "float32";
	case DATATYPE_FLOAT64: return "float64";
	case DATATYPE_UNDEFINED:
		/* no break */
	default:
		return "undefined";
	}
}

int ALDatatypeGetSize(ALDatatype type) {
	switch (type) {
	case DATATYPE_FLOAT32: return sizeof(float);
	case DATATYPE_FLOAT64: return sizeof(double);
	case DATATYPE_UNDEFINED:
		/* no break */
	default:
		// Shouldn't be asking the size of a undefined type, so abort for debugging
		assert(false);
		return -1;
	}
}
