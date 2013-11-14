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

#include "drawHairNode.h"
#include "ribGenCmd.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "RedBug", "7.0", "Any");
	status = plugin.registerNode( "drawHairNode", drawHairNode::id, &drawHairNode::creator, 
								&drawHairNode::initialize, MPxNode::kLocatorNode );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = plugin.registerCommand( "ribGenCmd", ribGenCmd::creator );
	if (!status) {
		status.perror("registerCommand");
		return status;
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( drawHairNode::id );
	if (!status) {
		status.perror("failed of degregister drawHairNode");
		return status;
	}

	status = plugin.deregisterCommand( "ribGenCmd" );
	if (!status) {
		status.perror("failed of deregister ribGenCmd");
		return status;
	}

	return status;
}
