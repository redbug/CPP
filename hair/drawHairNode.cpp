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
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <maya/MTypeId.h> 
#include <maya/MPoint.h>
#include <maya/MPlug.h>
#include <maya/MString.h> 
#include <maya/MVector.h>
#include <maya/MColor.h>
#include <maya/M3dView.h>
#include <maya/MDistance.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnTransform.h>
#include <maya/MPlugArray.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MFnTransform.h>
#include <maya/MVector.h>


MTypeId	drawHairNode::id( 0x80007 );
MObject	drawHairNode::input; 
MObject	drawHairNode::curveNumMO; 
MObject	drawHairNode::baseWidthMO;
MObject	drawHairNode::tipWidthMO;
MObject	drawHairNode::baseRandMO;
MObject	drawHairNode::tipRandMO;
MObject	drawHairNode::spreadMO;
MObject	drawHairNode::lengthRandMO;				
MObject	drawHairNode::normOffsetMO;
MObject	drawHairNode::smoothMO;
MObject	drawHairNode::widthAmpMO;
MObject	drawHairNode::heightAmpMO;
MObject	drawHairNode::curlMO;
MObject	drawHairNode::fadeMO;
MObject	drawHairNode::percentageMO;
MObject	drawHairNode::nodeIdMO;

int										drawHairNode::nodeId = 0;
nodeMap							drawHairNode::nodeManager;


drawHairNode::drawHairNode() {
	flag = false;
	helixResult = NULL;
}

void drawHairNode::initialNodeId(){
	MObject thisNode = thisMObject();			
	MPlug plug( thisNode, nodeIdMO );
	plug.setValue(nodeId++);
}

drawHairNode::~drawHairNode() {
	delete [] helixResult;
}

MStatus drawHairNode::compute( const MPlug&, MDataBlock& data )
{ 
	return MS::kUnknownParameter;
}

MPoint	drawHairNode::cubicRand(float range, unsigned int seed )
{
		int r = range * precision;
	
		if(r == 0){
			return MPoint();
		}
		else{
			srand(seed);
			float x = (rand()%(2*r)-r)/(float)precision;
			float y = (rand()%(2*r)-r)/(float)precision;
			float z = (rand()%(2*r)-r)/(float)precision;
			return MPoint(x, y, z);
		}
}

void drawHairNode::draw( M3dView & view, const MDagPath &, 
							 M3dView::DisplayStyle style,		
							 M3dView::DisplayStatus status )
{ 
	MStatus stats;

	if (!flag ){
		flag = true;
		initialNodeId();
	}

	// RETRIVE ATTRIBUTE VALUE
	MObject thisNode = thisMObject();			
	MPlug plug( thisNode, curveNumMO );

	plug.getValue( curveNum );

	plug.setAttribute( baseWidthMO );
	plug.getValue( baseWidth );

	plug.setAttribute( tipWidthMO );
	plug.getValue( tipWidth );

	plug.setAttribute( baseRandMO );
	plug.getValue( baseRand );

	plug.setAttribute( tipRandMO );
	plug.getValue( tipRand );

	plug.setAttribute( spreadMO );
	plug.getValue( spread );

	plug.setAttribute( lengthRandMO );
	plug.getValue( lengthRand );

	plug.setAttribute( normOffsetMO );
	plug.getValue( normOffset );	

	plug.setAttribute( smoothMO );
	plug.getValue( smooth );

	plug.setAttribute( percentageMO );
	plug.getValue( percentage );

	plug.setAttribute( curlMO );
	plug.getValue( curl );

	plug.setAttribute( widthAmpMO );
	plug.getValue( widthAmp );	

	plug.setAttribute( heightAmpMO );
	plug.getValue( heightAmp );

	MPlug inSurface( thisNode, input );
	if ( inSurface.isConnected() == true )
	{
		/*===========================*
		 * get the NurbsPlaneShape
		 *===========================*/
		MPlugArray srcPlugs;											
		inSurface.connectedTo( srcPlugs, true, false );
		MPlug srcPlug = srcPlugs[0];

		MObject surface = srcPlug.node();
		MFnNurbsSurface surfaceFn( surface );

		MDagPath path;
		surfaceFn.getPath( path );
		MMatrix matrixNPS = path.inclusiveMatrix();		

		//U:髮長, V:髮寬
		int cvNumU = surfaceFn.numCVsInU();
		int cvNumV = surfaceFn.numCVsInV();
	
		MPoint pt, prePt, rotatePt, rot, oldPt;								//used for spread
		double startU, endU, startV, endV, paramU, paramV;
		surfaceFn.getKnotDomain( startU, endU, startV, endV );
		
		//點與點之間的間距
		double Ustep, Vstep;
		Ustep = Vstep =0;
		
		if(cvNumU != 1)
			Ustep = (endU - startU) / (cvNumU-1);
		if(curveNum != 1)
			Vstep = (endV - startV) / (curveNum-1);

		float randStep;

		hairData foo( 1, curveNum, cvNumU );
				 

		int midPosition = curveNum/2 + 1;
		float factor1, factor2, factor3, tmp1;
		
		double degree, radian;
		degree = radian = 0.0;
		

		srand(500);
		
		for( int i=0; i < curveNum ; i++ )
		{	
			
			/*=========================================*
			 *	CALCULATE NORMAL OFFSET DISTANCE				
			 *=========================================*/
			paramV = Vstep * i ;																							//vertex's V componet
			
			MVector normal = surfaceFn.normal( 0, paramV, MSpace::kObject );		//normal vector of V: parallel axis Y
			normal.normalize();
			
			int r = normOffset * precision;
			float f;
			if (r==0)
				f=0.0;
			else if (r<0){
				f = (rand()%r)/(float)precision * -1;
			}
			else{
				f = (rand()%r)/(float)precision;
			}

			MPoint offset( normal.x*f, normal.y*f, normal.z*f );										//normal.x = normal.z = 0; 
		

			/*=========================================*
			 *	CALCULATE RANDOM OF LENGTH						
			 *=========================================*/
			float URandStep =  ( abs(1- lengthRand) + ((rand()%precision)/(float)precision) * lengthRand )* Ustep;

			tmp1 = (abs(i-midPosition)+1)/(float)curveNum;
			factor1 = pow( tmp1, 2 );																					//離中心點愈遠曲度愈大.
			factor2 = pow(1+URandStep, 2);																		//髮長愈長曲度愈大

			for( int j=0; j < cvNumU ; j++ )
			{			
				paramU = URandStep * j;																				//the value of U componet.

				surfaceFn.getPointAtParam( paramU, paramV, pt, MSpace::kObject);								//(u,v) -> pt (x, y, z)
				if(j>0){
					surfaceFn.getPointAtParam( URandStep * (j-1), paramV, prePt, MSpace::kObject);	
					rot = pt - prePt;
				}

			/*=========================================*
			 *	CALCULATE RANDOM OF baseRand and tipRand		  *
			 *=========================================*/
				float randRange = (j/(float)cvNumU * abs(tipRand)  +  (cvNumU - j)/(float)cvNumU * abs(baseRand))/10;
				MPoint tmpP = cubicRand(randRange, i+j+1);							//每個點對原curve的偏移量
				if(j==0){
					//pt.x += tmpP.x;
					pt.y += tmpP.y;
				}else{
					pt += tmpP;
				}

			/*=========================================*
			 *	CALCULATE SPREAD OF HAIR
			 *=========================================*/
				factor3 = 1+log((double)j);									//愈下面的點曲度愈大
				
				if( j >0 ){
					degree = spread * 100 * factor1 * factor2 * factor3 ;
					radian = degree * radianFactor;

					if( i < midPosition ){
						rotatePt.x = rot.z * sin(-radian) + rot.x * cos(-radian);
						rotatePt.z = rot.z * cos(-radian) - rot.x * sin(-radian);
					}else{
						rotatePt.x = rot.z * sin(radian) + rot.x * cos(radian);
						rotatePt.z = rot.z * cos(radian) - rot.x * sin(radian);
					}
					pt.x = oldPt.x + rotatePt.x;
					pt.z = oldPt.z + rotatePt.z;
				}

				
				oldPt = pt;
				pt += offset;														//整條頭髮上下偏移
				pt *= matrixNPS;												//每個點跟NurbsPlane's 一起 transformation
				
				//將(U,V)上的點資訊存入HairNode
				foo.pSurface[0].pCurve[i].points[j] = MPoint(pt.x, pt.y, pt.z);
			}
		}
	

		/*=========================================*
		 *	Computing Helix																					 *
		 *========================================= */
		ComputeHelix(foo);
		DrawHelix(view, true);
	}
	
}

//Change of base
float drawHairNode::myLog(float base, float real){
	return log(real)/log(base);
}

//Polynomial form of n-order Bezier curve.
MPoint drawHairNode::PointOnOrderNBezier( MPointArray& cp, float t ){
	unsigned length = cp.length();
	double *fact = new double[length];
	fact[0]=1;
	MPoint result;
	double c;

	for(unsigned i=1; i<length; i++){
		fact[i] = i * fact[i-1];
	}

	int n = length - 1;
	for(int i=0; i<length; ++i){
		c = fact[n]/(fact[i] * fact[n-i]);
		result += cp[i] * c * pow((1-t), n-i) * pow(t, i);
	}
	delete fact;
	return result;
}

//Compute the Bezier Curve.
void drawHairNode::ComputeBezier( MPointArray &cp, int numberOfPoints, MPointArray &curve)
{
    float   dt = 1.0 / ( numberOfPoints - 1 );
	MObject thisNode = thisMObject();			
	MPlug plug( thisNode, fadeMO );
	plug.getValue( fade );
	curve.append(cp[0].x, cp[0].y, cp[0].z);		//dummy vertex.

	for(int i = 0; i < numberOfPoints; i++){
		curve.append(PointOnOrderNBezier( cp, myLog(fade, 1+i*dt*(fade-1) ) ) ) ;
	}

	int last = cp.length()-1;
	curve.append(cp[last].x, cp[last].y, cp[last].z);	//dummy vertex.
}

//Computer the Helix
void drawHairNode::ComputeHelix(const hairData &foo){
	MPointArray curve, helix;
	if(helixResult != NULL)
		delete [] helixResult;

	helixResult = new MPointArray[curveNum];

	for( int i=0; i < curveNum ; ++i )
	{
		//將原curve上的點丟去算Bezier曲線, 並將結果存入curve
		ComputeBezier( foo.pSurface[0].pCurve[i].points, smooth, curve );
		
		double x, y, z;
		float dt = 1.0 / ( smooth - 1 );
		
		//computer the helix along a curve.
		for(int j=1; j <= smooth ; ++j){

			/* 座標轉換 */
			
			// 將曲線上的點轉為R(w,v,u)座標系

			// P'(s)	
			x = (curve[j].x - curve[j-1].x)/dt;
			y = abs(curve[j].y - curve[j-1].y)/dt;
			z = abs(curve[j].z - curve[j-1].z)/dt;
			
			// w = P'(s).
			MVector w(x, y, z);
			w.normalize();
	
			// P''(s)
			x = (curve[j+1].x - 2*curve[j].x + curve[j-1].x)/pow(dt,2);
			y = abs(curve[j+1].y - 2*curve[j].y + curve[j-1].y)/pow(dt,2);
			z = abs(curve[j+1].z - 2*curve[j].z + curve[j-1].z)/pow(dt,2);
		
			// u = P'(s) X P''(s).
			MVector u(w); 	
			u = u^MVector(x,y,z);
			u.normalize();

			// v = w X u
			MVector v= w^u;
			v.normalize();
			
			// 求helix上的點, 由R(w,v,u)空間座標 --> R(x,y,z)空間座標 
			MVector result( /* w + */	
							heightAmp * sin( j * dt * curl )  * u + 
					        widthAmp * cos( j * dt * curl )  * v );
			
			//local座標 --> global座標
			result += curve[j];
			helix.append(result);
		}
		helixResult[i] = helix;
		curve.clear();
		helix.clear();	
	} 	


	/* put the helixResult of every drawHairNode into nodeManager which is a map */
	static vector<MPointArray> helixResultVec(helixResult, helixResult+curveNum);

	int nId;
	MObject thisNode = thisMObject();			
	MPlug plug( thisNode, nodeIdMO );
	plug.getValue( nId );

	helixResultVec.clear();
	helixResultVec.assign(helixResult, helixResult+curveNum);
	nodeManager[nId] = helixResultVec;

}

//Draw the Helix, the second parameter is a flag that can switch to draw a line or draw  points.
void drawHairNode::DrawHelix(M3dView &view, bool continuity){
	//the below two variables control the percentage of line will draw.
	float step = 100 / percentage;				
	float rStep=0.0;

	view.beginGL();
	for( int i=0; i < curveNum; i = floor((rStep += step)))
	{
		if(continuity == true)
			glBegin( GL_LINES );	
		else
			glBegin( GL_POINTS );

		//draw curve
/*			glColor3f(1,1,1);
		for(int j=0; j < curve.length()-1 ; ++j){
			glVertex3f( curve[j].x, curve[j].y, curve[j].z );
			glVertex3f( curve[j+1].x, curve[j+1].y, curve[j+1].z );
		}
*/
		glColor3f(0.1, 0.2, 0.333);

		for(int j=0; j < helixResult[i].length()-1 ; ++j){
			glVertex3f( helixResult[i][j].x, helixResult[i][j].y, helixResult[i][j].z );
			glVertex3f( helixResult[i][j+1].x, helixResult[i][j+1].y, helixResult[i][j+1].z );
		}
		
		glEnd();
	}
	view.endGL();	
}

bool drawHairNode::isBounded() const
{ 
	return true;
}

MBoundingBox drawHairNode::boundingBox() const
{    
	MPoint corner1( -0.17, 0.0, -0.7 );
	MPoint corner2( 0.17, 0.0, 0.3 );

	corner1 = corner1;
	corner2 = corner2;

	return MBoundingBox( corner1, corner2 );
}


void* drawHairNode::creator()
{
	return new drawHairNode();
}

MStatus drawHairNode::initialize()
{ 	
	MFnNumericAttribute nAttr;
	MFnTypedAttribute typAttr;
	MStatus				stats;

	input = typAttr.create( "input", "in", MFnData::kNurbsSurface );	
	typAttr.setHidden( true );
	addAttribute( input );
	
	curveNumMO = nAttr.create( "number", "n", MFnNumericData::kInt );	
	nAttr.setStorable( true );
	nAttr.setDefault( 50 );
	addAttribute( curveNumMO );

	baseWidthMO = nAttr.create( "baseWidth", "bw", MFnNumericData::kFloat );	
	nAttr.setStorable( true );
	nAttr.setDefault( 0.1 );
	nAttr.setMin(0.01);
	nAttr.setMax(1.0);
	addAttribute( baseWidthMO );

	tipWidthMO = nAttr.create( "tipWidth", "tw", MFnNumericData::kFloat );	
	nAttr.setStorable( true );
	nAttr.setDefault( 0.01 );
	nAttr.setMin(0.01);
	nAttr.setMax(1.0);
	addAttribute( tipWidthMO );
	
	baseRandMO = nAttr.create( "baseRand", "br", MFnNumericData::kFloat );	
	nAttr.setStorable( true );
	nAttr.setDefault( 0.0 );
	nAttr.setMin(0.0);
	nAttr.setMax(50.0);
	addAttribute( baseRandMO );

	tipRandMO = nAttr.create( "tipRand", "tr", MFnNumericData::kFloat );	
	nAttr.setStorable( true );
	nAttr.setDefault( 0.0 );
	nAttr.setMin(0.0);
	nAttr.setMax(50.0);
	addAttribute( tipRandMO );

	spreadMO = nAttr.create( "spread", "s", MFnNumericData::kFloat );	
	nAttr.setStorable( true );
	nAttr.setDefault( 0.0 );
	addAttribute( spreadMO );

	lengthRandMO = nAttr.create( "lengthRand", "lr", MFnNumericData::kFloat );	
	nAttr.setStorable( true );
	nAttr.setDefault( 0.0 );
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	addAttribute( lengthRandMO );

	normOffsetMO = nAttr.create( "normOffset", "no", MFnNumericData::kFloat );	
	nAttr.setStorable( true );
	nAttr.setDefault( 0.0 );
	addAttribute( normOffsetMO );

	smoothMO = nAttr.create("smooth", "sn", MFnNumericData::kInt);
	nAttr.setStorable( true );
	nAttr.setDefault(10);
	nAttr.setMin(10);
	nAttr.setMax(650);
	addAttribute( smoothMO );

	widthAmpMO = nAttr.create("widthAmp", "wa", MFnNumericData::kFloat);
	nAttr.setStorable( true );
	nAttr.setDefault( 0.0 );
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	addAttribute( widthAmpMO );

	heightAmpMO = nAttr.create("heightAmp", "ha", MFnNumericData::kFloat);
	nAttr.setStorable( true );
	nAttr.setDefault( 0.0 );
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);
	addAttribute( heightAmpMO );
	
	curlMO = nAttr.create("curl", "sm", MFnNumericData::kInt);
	nAttr.setStorable( true );
	nAttr.setDefault( 0 );
	nAttr.setMin(0);
	addAttribute( curlMO );

	fadeMO = nAttr.create("fade", "fa", MFnNumericData::kFloat);
	nAttr.setStorable( true );
	nAttr.setDefault( 2.0 );
	nAttr.setMin( 2.0 );
	nAttr.setMax( 100.0 );
	addAttribute( fadeMO );

	percentageMO = nAttr.create("percentage", "pc", MFnNumericData::kFloat);
	nAttr.setStorable( true );
	nAttr.setDefault( 100.0 );
	nAttr.setMin( 1.0 );
	nAttr.setMax( 100.0 );
	addAttribute( percentageMO );

	nodeIdMO = nAttr.create("nodeId", "nid", MFnNumericData::kInt);
	nAttr.setStorable( true );
	nAttr.setHidden(true);
	addAttribute(nodeIdMO);

	return MS::kSuccess;
}