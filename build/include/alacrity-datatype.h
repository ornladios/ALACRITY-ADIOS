/*
 * alacrity-datatype.h
 *
 *  Created on: Nov 19, 2012
 *      Author: David A. Boyuka II
 */

#ifndef ALACRITY_DATATYPE_H_
#define ALACRITY_DATATYPE_H_

#include <stdbool.h>

#include <alacrity-types.h>

_Bool ALDatatypeIsValid(ALDatatype type);
_Bool ALDatatypeIsDefined(ALDatatype type);
const char * ALDatatypeGetName(ALDatatype type);
int ALDatatypeGetSize(ALDatatype type);

#endif /* ALACRITY_DATATYPE_H_ */
