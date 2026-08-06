/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <cmath>
#include <memory>
#include "WarpMesh.h"


namespace intel_vvp_warp
{

struct Barycentric
{
	float _alpha;
	float _beta;
	float _gamma;
};

template<typename T>
class Triangle;

template<typename T>
class Point
{
	friend class Triangle<T>;
public:
	Point():
		_x(T()), _y(T())
	{
	}

	Point(const T& x, const T& y):
		_x(x), _y(y)
	{
	}

	~Point()
	{
	}

	T GetX()const
	{
		return _x;
	}
	T GetY()const
	{
		return _y;
	}

	void SetX(const T& x)
	{
		_x=x;
	}
	void SetY(const T& y)
	{
		_y=y;
	}

public:
	T _x;
	T _y;
};


template<typename T>
class Triangle;

typedef std::shared_ptr<Triangle<int> > TranslatedTrianglePtr;


template<typename T>
class Triangle
{
public:
	Triangle()
	{
	}

	Triangle(const Point<T>& p1, const Point<T>& p2, const Point<T>& p3):
		_p1(p1), _p2(p2), _p3(p3),
		_p_rect_min(p1),
		_p_rect_max(p1)
	{
		CheckUpdateMinMax(p2);
		CheckUpdateMinMax(p3);
	}

	virtual ~Triangle()
	{
	}

	TranslatedTrianglePtr Translate(uint32_t width, uint32_t height)
	{
		Point<int> p1( _p1.GetX() * width, _p1.GetY() * height);
		Point<int> p2( _p2.GetX() * width, _p2.GetY() * height);
		Point<int> p3( _p3.GetX() * width, _p3.GetY() * height);

		TranslatedTrianglePtr pt( new Triangle<int>(p1, p2, p3) );
		return pt;
	}

	void InterpolateEx(const Triangle<int>& t, WarpMesh& mesh, int width, int height) const
	{
		const uint32_t mesh_step = mesh.GetStep();
		const uint32_t mesh_frac_prec = mesh.GetFractBits();

		int align_val = mesh_step << mesh_frac_prec;
		int grid_step = mesh_step << mesh_frac_prec;

		int x = _p_rect_min.GetX();
		x = ( ( x + ( align_val - 1 ) ) & ~( align_val - 1 ) ) - align_val;	// roundup to 8x towards zero;
		int y = _p_rect_min.GetY();
		y = ( ( y + ( align_val - 1 ) ) & ~( align_val - 1 ) ) - align_val;	// roundup to 8x towards zero;

		if(x<0) x = 0;
		if(y<0) y = 0;

		int xmax = ( (_p_rect_max.GetX() > width) ? width : _p_rect_max.GetX() );
		int ymax = ( (_p_rect_max.GetY() > height) ? height : _p_rect_max.GetY() );

		while(y <= ymax)
		{
			int tmpx = x;
			while(tmpx <= xmax)
			{
				Barycentric pb;
				GetBarycentric(Point<int>(tmpx, y), pb);

				bool outside = (pb._alpha < 0.0f) || (pb._beta < 0.0f) || (pb._gamma < 0.0f);

				if(!outside)
				{
					int xt = (int)(pb._alpha * t._p1._x + pb._beta * t._p2._x + pb._gamma * t._p3._x);
					int yt = (int)(pb._alpha * t._p1._y + pb._beta * t._p2._y + pb._gamma * t._p3._y);

					uint32_t yidx = (y >> mesh_frac_prec) / mesh_step;
					uint32_t xidx = (tmpx >> mesh_frac_prec) / mesh_step;

					mesh_node_t* node = mesh.GetRow(yidx);
					node[xidx]._x = xt;
					node[xidx]._y = yt;
				}

				tmpx += grid_step;
			}
			y += grid_step;
		}

		return;
	}

public:
	Point<T> _p1;
	Point<T> _p2;
	Point<T> _p3;

	Point<T> _p_rect_min;
	Point<T> _p_rect_max;

	void CheckUpdateMinMax(const Point<T>& p)
	{
		if( p.GetX() <  _p_rect_min.GetX() )
			_p_rect_min.SetX( p.GetX() );
		else if( p.GetX() >  _p_rect_max.GetX() )
			_p_rect_max.SetX( p.GetX() );

		if( p.GetY() <  _p_rect_min.GetY() )
			_p_rect_min.SetY( p.GetY() );
		else if( p.GetY() >  _p_rect_max.GetY() )
			_p_rect_max.SetY( p.GetY() );
	}

	void GetBarycentric(const Point<T>& p, Barycentric& pb) const
	{
		float det = static_cast<float>((_p2._y - _p3._y) * (_p1._x - _p3._x) + (_p3._x - _p2._x) * (_p1._y - _p3._y));

		pb._alpha = ((_p2._y - _p3._y) * (p._x - _p3._x) + (_p3._x - _p2._x) * (p._y - _p3._y)) / det;
		pb._beta = ((_p3._y - _p1._y) * (p._x - _p3._x) + (_p1._x - _p3._x) * (p._y - _p3._y)) / det;
		pb._gamma = 1.0f - pb._alpha - pb._beta;

		return;
	}
};

} // namespace intel_vvp_warp
