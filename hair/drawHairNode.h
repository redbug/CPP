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

#ifndef _DRAW_HAIR_NODE_H
#define _DRAW_HAIR_NODE_H

#define PI 3.14159265

#include "hairData.h"
#include <map>
#include <vector>
#include <maya/MPxLocatorNode.h>

using namespace std;

typedef map<int, vector<MPointArray> > nodeMap;
typedef nodeMap::value_type valueType;


static const double radianFactor = PI/180;
static const int precision = 1000;

class drawHairNode : public MPxLocatorNode
{
public:
	/* member function */
	drawHairNode();
	virtual ~drawHairNode(); 
    virtual MStatus  			compute( const MPlug& plug, MDataBlock& data );
	virtual void				draw( M3dView & view, const MDagPath & path, 
													M3dView::DisplayStyle style,
													M3dView::DisplayStatus status );
	virtual bool				isBounded() const;
	virtual MBoundingBox	boundingBox() const;	
	static  void *            creator();
	static  MStatus           initialize();


	/* data member */
	static	MTypeId		id;										//Node id for maya.
	static 	MObject		input;									//the attribute connected with nurbsPlane.
	static 	MObject		curveNumMO;					//the number of curve.
	static	MObject		baseWidthMO;					//the width of hair base.
	static	MObject		tipWidthMO;					//the width of hair tip.
	static	MObject		baseRandMO;					//the degree of randomness of the hair base.
	static	MObject		tipRandMO;						//the degree of randomness of the hair tip.
	static	MObject		spreadMO;						//the degree of spread of the hair.
	static	MObject		lengthRandMO;				//the degree of randomness of the hair length.
	static	MObject		normOffsetMO;				//the normal offset of the hair.
	static	MObject		smoothMO;						//the number of vertexes of the curve.
	static	MObject		widthAmpMO;					//the width amplitude of the helixes.
	static	MObject		heightAmpMO;				//the heigth amplitude of the helixes.
	static	MObject		curlMO;								//the degree of curly of the curve.
	static	MObject		fadeMO;								//the smoothness of the hair base.
	static	MObject		percentageMO;				//the percentage of drawing hair.
	static	MObject		nodeIdMO;						//node id for mapping to  the drawHairNode.
	
	static int nodeId;
	static nodeMap nodeManager;						//mapping the nodes to the hair their own.
	
	int	 curveNum;
	float baseWidth;
	float tipWidth;
	float baseRand;
	float tipRand;
	float spread;
	float lengthRand;
	float normOffset;
	float smooth;
	float widthAmp;
	float heightAmp;
	float curl;
	float fade;
	float percentage;
	bool flag;


private:
	MPoint	cubicRand(float, unsigned int );											//點的隨機偏移.
	void		ComputeBezier( MPointArray&, int, MPointArray& );		//計算Bezier curve.
	MPoint	PointOnOrderNBezier( MPointArray&, float );					//n-order Bezier curve.
	float		myLog(float, float);																	//換底.
	void		ComputeHelix(const hairData&);											//計算Helix.
	void		DrawHelix(M3dView&, bool continuity = true);				//畫Helix.
	void		initialNodeId();																			//給每個node一個id, 供nodeManager管理.

	MPointArray *helixResult; 																		//儲存hair的計算結果.
};


#endif


