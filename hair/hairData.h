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

#ifndef _HAIR_DATA_H
#define _HAIR_DATA_H

#include <maya/MPointArray.h>

struct curveData
{	
	MPointArray points;
};

struct surfaceData
{
	curveData *pCurve;
	~surfaceData()  { delete [] pCurve ; };
};

struct hairData
{
	surfaceData *pSurface;
	hairData( int surfaceNum, int curveNum , int pointNum )
	{
		pSurface = new surfaceData[ surfaceNum ];
		for (  int i = 0 ; i < surfaceNum ; i++ ) 
		{
			pSurface[i].pCurve = new curveData[ curveNum ];
			for (  int j = 0 ; j < curveNum ; j++ )				
				pSurface[i].pCurve[j].points = MPointArray(pointNum);
		}
	}
	~hairData()  { delete [] pSurface ; };
};
#endif

