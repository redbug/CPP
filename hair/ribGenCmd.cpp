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

#include "ribGenCmd.h"

#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MStatus.h>
#include <maya/MPointArray.h>


MStatus ribGenCmd::doIt( const MArgList& args)
{
	/*=========================================*
	  *		the parameters of command															
	  *	=========================================*/
	MString ribPath, shaderPath;					
	unsigned index;
	index = args.flagIndex("p", "ribPath");

	if(MArgList::kInvalidArgIndex != index)
		args.get(index+1, ribPath);
	
	index = args.flagIndex("sp", "shaderPath");
	if(MArgList::kInvalidArgIndex != index)
		args.get(index+1, shaderPath);

	/*=========================================*
	  *		shaderPath & ribPath																		
	  *=========================================*/
	RtToken shader= new char[50] , path=new char[50];
	strcpy(shader, shaderPath.asChar());
	strcpy(path, ribPath.asChar());

	char *curve[] = {"curves"};	

	RtMatrix identityMatrix =
	{	{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }}; 
	
	RtInt ustep, vstep;
	ustep = vstep =1;

  /*=====================================*
	*		Begenning of writting out the .rib file.														
	*=====================================*/	
	RiBegin(path);
		RiAttributeBegin();
			RiTransformBegin();
				//RiSurface(shader);
			RiTransformEnd();


			//RiAttribute("identifier", "name", curve);
			RiConcatTransform(identityMatrix);
			RiShadingInterpolation(RI_SMOOTH);
			RiBasis(RiBezierBasis, ustep, RiBezierBasis, vstep);			



	int nodeId	= 0, knotNum = 0;
	float baseWidth = 0.0f, tipWidth	= 0.0f;
	MObject surface, node;
	/*=========================================*
	  *		get the informations of selected Objects in scene.																		
	  *=========================================*/	
	MSelectionList selection;
	MGlobal::getActiveSelectionList(selection);
	MItSelectionList iter(selection, MFn::kNurbsSurface );

	for(; !iter.isDone(); iter.next())
	{
		RtInt numCurves = 0;
		RtInt numVertexs = 0;

		/*======================================*
		  *		get the drawHairNode from selected NurbsSurface. 
		  *======================================*/
		iter.getDependNode(surface);
		MFnDependencyNode surfaceFn(surface);
		MStatus state;
		MPlug plug = surfaceFn.findPlug("worldSpace",false, &state);

		plug = plug.elementByLogicalIndex(0);
		MPlugArray desPlugs;
		plug.connectedTo(desPlugs,false,true);
		plug = desPlugs[0];
		node = plug.node();				//drawHairNode has found here!!

		/*=====================================*
		  *		get the attributes of drawHairNode. 
		  *=====================================*/
		MFnDependencyNode hairNodeFn(node);
		plug = hairNodeFn.findPlug("nodeId");
		plug.getValue(nodeId);
		MGlobal::displayInfo(MString(" nodeId: ")+nodeId);

		plug = hairNodeFn.findPlug("number");
		plug.getValue(numCurves);

		plug= hairNodeFn.findPlug("smooth");
		plug.getValue(knotNum);

		plug = hairNodeFn.findPlug("baseWidth");
		plug.getValue(baseWidth);

		plug = hairNodeFn.findPlug("tipWidth");
		plug.getValue(tipWidth);

		/*=====================================*
		  *		caculate the linear interpolate of the width of the curve.
		  *=====================================*/
		numVertexs = numCurves * knotNum;
		int widthNum = numCurves * (knotNum -2 );
		float *curveWidth = new float[widthNum];
		
		float widthStep = 0.0f;
		for(int c=0; c<widthNum; ++c){
			widthStep = ((c%(knotNum-2)) / (float)(knotNum-3)) *(tipWidth - baseWidth);
			if(widthStep < epslion)
				widthStep = 0.0f;
			curveWidth[c] = baseWidth + widthStep;
		}
		

		RtInt *nvertices = new RtInt[numCurves];						//the numbers of vertices on each curve.
		RtPoint *vertexs = new RtPoint[numVertexs];				//the total vertexs.

		/*=====================================*
		  *		nvertices[] assignment.														
		  *=====================================*/
		for(int j=0; j<numCurves ; ++j){
			nvertices[j] = knotNum;
		}



		/*=====================================*
		  *		get the hair's datas from the static member 
		  *		named "nodeManager" of the drawHairNode class. 
		  *=====================================*/
		nodeMap::iterator  iter =  drawHairNode::nodeManager.find(nodeId);
		vector<MPointArray> helixVec = iter->second;
		

		/*=====================================*
		  *		vertexs[] assignment.														
		  *=====================================*/	
		float x=0, y=0, z=0;
		int countVT=0;
		for(vector<MPointArray>::iterator it=helixVec.begin();  it != helixVec.end();  ++it){
			MPointArray helixCurve = (MPointArray)(*it);

			for(int k=0; k <  helixCurve.length() ; ++k){
				x = helixCurve[k].x;
				if(fabs(x) < epslion)
					vertexs[countVT][0] = 0;
				else
					vertexs[countVT][0] = helixCurve[k].x;
			
				y = helixCurve[k].y;
				if(fabs(y) < epslion)
					vertexs[countVT][1] = 0;
				else
					vertexs[countVT][1] = helixCurve[k].y;
			
				z = helixCurve[k].z;			
				if(fabs(z) < epslion)
					vertexs[countVT++][2] = 0;
				else
					vertexs[countVT++][2] = helixCurve[k].z;
			}
		}
		RiCurves( RI_CUBIC, numCurves, nvertices, RI_NONPERIODIC, RI_P, vertexs, RI_WIDTH, curveWidth, RI_NULL);
	}
	


		RiAttributeEnd();
	RiEnd();

	return redoIt();
}


MStatus ribGenCmd::redoIt(){
	return dgMod.doIt();
}

MStatus ribGenCmd::undoIt()
{
    MGlobal::displayInfo(MString("ribGenCmd command undone!\n") );
	return dgMod.undoIt();
}

void* ribGenCmd::creator()
{
	return new ribGenCmd();
}

ribGenCmd::ribGenCmd()
{}

ribGenCmd::~ribGenCmd()
{}

bool ribGenCmd::isUndoable() const
{
	return true;
}

