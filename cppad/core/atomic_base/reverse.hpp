# ifndef CPPAD_CORE_ATOMIC_BASE_REVERSE_HPP
# define CPPAD_CORE_ATOMIC_BASE_REVERSE_HPP

/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-18 Bradley M. Bell

CppAD is distributed under multiple licenses. This distribution is under
the terms of the
                    Eclipse Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */
/*
$begin atomic_reverse$$
$spell
	sq
	mul.hpp
	afun
	ty
	px
	py
	Taylor
	const
	CppAD
$$

$section Atomic Reverse Mode$$
$spell
	bool
$$

$head Syntax$$
$icode%ok% = %afun%.reverse(%q%, %tx%, %ty%, %px%, %py%)%$$

$head Purpose$$
This function is used by $cref/reverse/Reverse/$$
to compute derivatives.

$head Implementation$$
If you are using
$cref/reverse/Reverse/$$ mode,
this virtual function must be defined by the
$cref/atomic_user/atomic_ctor/atomic_user/$$ class.
It can just return $icode%ok% == false%$$
(and not compute anything) for values
of $icode q$$ that are greater than those used by your
$cref/reverse/Reverse/$$ mode calculations.

$head q$$
The argument $icode q$$ has prototype
$codei%
	size_t %q%
%$$
It specifies the highest order Taylor coefficient that
computing the derivative of.

$head tx$$
The argument $icode tx$$ has prototype
$codei%
	const CppAD::vector<%Base%>& %tx%
%$$
and $icode%tx%.size() == (%q%+1)*%n%$$.
For $latex j = 0 , \ldots , n-1$$ and $latex k = 0 , \ldots , q$$,
we use the Taylor coefficient notation
$latex \[
\begin{array}{rcl}
	x_j^k    & = & tx [ j * ( q + 1 ) + k ]
	\\
	X_j (t)  & = & x_j^0 + x_j^1 t^1 + \cdots + x_j^q t^q
\end{array}
\] $$
Note that superscripts represent an index for $latex x_j^k$$
and an exponent for $latex t^k$$.
Also note that the Taylor coefficients for $latex X(t)$$ correspond
to the derivatives of $latex X(t)$$ at $latex t = 0$$ in the following way:
$latex \[
	x_j^k = \frac{1}{ k ! } X_j^{(k)} (0)
\] $$

$head ty$$
The argument $icode ty$$ has prototype
$codei%
	const CppAD::vector<%Base%>& %ty%
%$$
and $icode%tx%.size() == (%q%+1)*%m%$$.
For $latex i = 0 , \ldots , m-1$$ and $latex k = 0 , \ldots , q$$,
we use the Taylor coefficient notation
$latex \[
\begin{array}{rcl}
	Y_i (t)  & = & f_i [ X(t) ]
	\\
	Y_i (t)  & = & y_i^0 + y_i^1 t^1 + \cdots + y_i^q t^q + o ( t^q )
	\\
	y_i^k    & = & ty [ i * ( q + 1 ) + k ]
\end{array}
\] $$
where $latex o( t^q ) / t^q \rightarrow 0$$ as $latex t \rightarrow 0$$.
Note that superscripts represent an index for $latex y_j^k$$
and an exponent for $latex t^k$$.
Also note that the Taylor coefficients for $latex Y(t)$$ correspond
to the derivatives of $latex Y(t)$$ at $latex t = 0$$ in the following way:
$latex \[
	y_j^k = \frac{1}{ k ! } Y_j^{(k)} (0)
\] $$


$head F$$
We use the notation $latex \{ x_j^k \} \in B^{n \times (q+1)}$$ for
$latex \[
	\{ x_j^k \W{:} j = 0 , \ldots , n-1, k = 0 , \ldots , q \}
\]$$
We use the notation $latex \{ y_i^k \} \in B^{m \times (q+1)}$$ for
$latex \[
	\{ y_i^k \W{:} i = 0 , \ldots , m-1, k = 0 , \ldots , q \}
\]$$
We define the function
$latex F : B^{n \times (q+1)} \rightarrow B^{m \times (q+1)}$$ by
$latex \[
	y_i^k = F_i^k [ \{ x_j^k \} ]
\] $$
Note that
$latex \[
	F_i^0 ( \{ x_j^k \} ) = f_i ( X(0) )  = f_i ( x^0 )
\] $$
We also note that
$latex F_i^\ell ( \{ x_j^k \} )$$ is a function of
$latex x^0 , \ldots , x^\ell$$
and is determined by the derivatives of $latex f_i (x)$$
up to order $latex \ell$$.


$head G, H$$
We use $latex G : B^{m \times (q+1)} \rightarrow B$$
to denote an arbitrary scalar valued function of $latex \{ y_i^k \}$$.
We use $latex H : B^{n \times (q+1)} \rightarrow B$$
defined by
$latex \[
	H ( \{ x_j^k \} ) = G[ F( \{ x_j^k \} ) ]
\] $$

$head py$$
The argument $icode py$$ has prototype
$codei%
	const CppAD::vector<%Base%>& %py%
%$$
and $icode%py%.size() == m * (%q%+1)%$$.
For $latex i = 0 , \ldots , m-1$$, $latex k = 0 , \ldots , q$$,
$latex \[
	py[ i * (q + 1 ) + k ] = \partial G / \partial y_i^k
\] $$

$subhead px$$
The $icode px$$ has prototype
$codei%
	CppAD::vector<%Base%>& %px%
%$$
and $icode%px%.size() == n * (%q%+1)%$$.
The input values of the elements of $icode px$$
are not specified (must not matter).
Upon return,
for $latex j = 0 , \ldots , n-1$$ and $latex \ell = 0 , \ldots , q$$,
$latex \[
\begin{array}{rcl}
px [ j * (q + 1) + \ell ] & = & \partial H / \partial x_j^\ell
\\
& = &
( \partial G / \partial \{ y_i^k \} ) \cdot
	( \partial \{ y_i^k \} / \partial x_j^\ell )
\\
& = &
\sum_{k=0}^q
\sum_{i=0}^{m-1}
( \partial G / \partial y_i^k ) ( \partial y_i^k / \partial x_j^\ell )
\\
& = &
\sum_{k=\ell}^q
\sum_{i=0}^{m-1}
py[ i * (q + 1 ) + k ] ( \partial F_i^k / \partial x_j^\ell )
\end{array}
\] $$
Note that we have used the fact that for $latex k < \ell$$,
$latex \partial F_i^k / \partial x_j^\ell = 0$$.

$head ok$$
The return value $icode ok$$ has prototype
$codei%
	bool %ok%
%$$
If it is $code true$$, the corresponding evaluation succeeded,
otherwise it failed.

$children%
	example/atomic/reverse.cpp
%$$
$head Examples$$
The file $cref atomic_reverse.cpp$$ contains an example and test
that uses this routine.
It returns true if the test passes and false if it fails.

$end
-----------------------------------------------------------------------------
*/

namespace CppAD { // BEGIN_CPPAD_NAMESPACE
/*!
\file atomic_base/reverse.hpp
Atomic reverse mode.
*/
/*!
Link from reverse mode sweep to users routine.

\param q [in]
highest order for this reverse mode calculation.

\param tx [in]
Taylor coefficients corresponding to \c x for this calculation.

\param ty [in]
Taylor coefficient corresponding to \c y for this calculation

\param px [out]
Partials w.r.t. the \c x Taylor coefficients.

\param py [in]
Partials w.r.t. the \c y Taylor coefficients.

See atomic_reverse mode use documentation
*/
template <typename Base>
bool atomic_base<Base>::reverse(
	size_t                    q  ,
	const vector<Base>&       tx ,
	const vector<Base>&       ty ,
	      vector<Base>&       px ,
	const vector<Base>&       py )
{	return false; }

} // END_CPPAD_NAMESPACE
# endif
