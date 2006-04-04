/* -----------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-06 Bradley M. Bell

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
------------------------------------------------------------------------ */

/*
$begin CompareChange.cpp$$
$spell
	Cpp
$$

$section CompareChange Function: Example and Test$$

$index compare, change$$
$index example, CompareChange$$
$index test, CompareChange$$

$code
$verbatim%Example/CompareChange.cpp%0%// BEGIN PROGRAM%// END PROGRAM%1%$$
$$

$end
*/
// BEGIN PROGRAM

# include <CppAD/CppAD.h>

namespace { // put this function in the empty namespace
	template <typename Type>
	Type Minimum(const Type &x, const Type &y)
	{	// Use a comparision to compute the min(x, y)
		// (note that CondExp would never require retaping). 
		if( x < y )  
			return x;
		return y;
	}
}

bool CompareChange(void)
{	bool ok = true;
	using CppAD::AD;
	using CppAD::ADFun;
	using CppAD::Independent;

	// domain space vector
	size_t n = 2;
	CppADvector< AD<double> > X(n);
	X[0] = 3.;
	X[1] = 4.;

	// declare independent variables and start tape recording
	CppAD::Independent(X);

	// range space vector
	size_t m = 1;
	CppADvector< AD<double> > Y(m);
	Y[0] = Minimum(X[0], X[1]);

	// create f: x -> y and stop tape recording
	// use a pointer so that we can retape
	ADFun<double> *f;
	f = new ADFun<double>(X, Y);

	// evaluate zero mode Forward where conditional has the same result
	// note that f->CompareChange is not defined when NDEBUG is true
	CppADvector<double> x(n);
	CppADvector<double> y(m);
	x[0] = 3.5;
	x[1] = 4.;  
	y    = f->Forward(0, x);
	ok  &= (y[0] == x[0]);
	ok  &= (y[0] == Minimum(x[0], x[1]));
	ok  &= (f->CompareChange() == 0);

	// evaluate zero mode Forward where conditional has different result
	x[0] = 4.;
	x[1] = 3.;
	y    = f->Forward(0, x);
	ok  &= (y[0] == x[0]);
	ok  &= (y[0] != Minimum(x[0], x[1]));
	ok  &= (f->CompareChange() == 1); 

	// re-tape to obtain the new AD operation sequence
	delete f;    // free memory corresponding to previous new
	X[0] = 4.;
	X[1] = 3.;
	Independent(X);
	Y[0] = Minimum(X[0], X[1]);
	f    = new ADFun<double>(X, Y);

	// evaluate the function at new argument values
	y    = f->Forward(0, x);
	ok  &= (y[0] == x[1]);
	ok  &= (y[0] == Minimum(x[0], x[1]));
	ok  &= (f->CompareChange() == 0); 

	delete f;   // free memory corresponding to previous new
	return ok;
}


// END PROGRAM
