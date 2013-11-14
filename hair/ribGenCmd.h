/* Date: 2007/09/13 */
/*
** Copyright (c) 2000-2007 Sofa Studio.  All rights reserved.  This program or
** documentation contains proprietary confidential information and trade
** secrets of Sofa Studio.  Reverse engineering of object code is prohibited.
** Use of copyright notice is precautionary and does not imply
** publication.
**
** Author: Jack & RedBug
** E-mail: l314kimo@gmail.com
*/

#ifndef _RIB_GEN_CMD_H
#define _RIB_GEN_CMD_H

#include <math.h>

#include "ri.h"
#include "drawHairNode.h"

#include <maya/MPxCommand.h>
#include <maya/MDGModifier.h>
#include <maya/MString.h>

#define epslion 0.000000001

class MArgList;

class ribGenCmd : public MPxCommand
{

public:
				ribGenCmd();
	virtual		~ribGenCmd();

	MStatus		doIt( const MArgList& );
	MStatus		redoIt();
	MStatus		undoIt();
	bool		isUndoable() const;
	static		void* creator();

private:
	MDGModifier dgMod;
};

#endif