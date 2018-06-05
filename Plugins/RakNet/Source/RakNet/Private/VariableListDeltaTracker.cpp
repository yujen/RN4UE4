/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "VariableListDeltaTracker.h"

#include "RakNetPrivatePCH.h"


using namespace RakNet;

VariableListDeltaTracker::VariableListDeltaTracker() {nextWriteIndex=0;}
VariableListDeltaTracker::~VariableListDeltaTracker()
{
	unsigned int i;
	for (i=0; i < variableList.Size(); i++)
		rakFree_Ex(variableList[i].lastData,_FILE_AND_LINE_);
}

// Call before using a series of WriteVar
void VariableListDeltaTracker::StartWrite(void) {nextWriteIndex=0;}

void VariableListDeltaTracker::FlagDirtyFromBitArray(unsigned char *bArray)
{
	unsigned short readOffset=0;
	for (readOffset=0; readOffset < variableList.Size(); readOffset++)
	{
		bool result = ( bArray[ readOffset >> 3 ] & ( 0x80 >> ( readOffset & 7 ) ) ) !=0;

		if (result==true)
			variableList[readOffset].isDirty=true;
	}
}
