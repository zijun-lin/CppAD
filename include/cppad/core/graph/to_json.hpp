# ifndef CPPAD_CORE_GRAPH_TO_JSON_HPP
# define CPPAD_CORE_GRAPH_TO_JSON_HPP

/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-19 Bradley M. Bell

CppAD is distributed under the terms of the
             Eclipse Public License Version 2.0.

This Source Code may also be made available under the following
Secondary License when the conditions for such availability set forth
in the Eclipse Public License, Version 2.0 are satisfied:
      GNU General Public License, Version 2.0 or later.
---------------------------------------------------------------------------- */

# include <cppad/core/ad_fun.hpp>
# include <cppad/local/op_code_dyn.hpp>
# include <cppad/local/json/operator.hpp>
# include <cppad/local/json/writer.hpp>

/*
------------------------------------------------------------------------------
$begin to_json$$
$spell
    Json
    cpp
$$

$section Create a Json AD Graph Corresponding to an ADFun Object$$

$head Under Construction$$
This routine is under construction and subject to
change without backward compatibility.

$head Syntax$$
$codei%
    %graph% = %fun%.to_json()
%$$

$head Prototype$$
$srcfile%include/cppad/core/graph/to_json.hpp%
    0%// BEGIN_PROTOTYPE%// END_PROTOTYPE%1
%$$

$head fun$$
is the $cref/ADFun/adfun/$$ object.

$head graph$$
The return value of $icode graph$$ is a
$cref json_ad_graph$$ representation of the corresponding function.

$head Base$$
is the type corresponding to this $cref/ADFun/adfun/$$ object;
i.e., its calculations are done using the type $icode Base$$.

$head RecBase$$
in the prototype above, $icode RecBase$$ is the same type as $icode Base$$.

$children%
    example/json/to_json.cpp
%$$
$head Example$$
The file $cref to_json.cpp$$ is an example and test of this operation.

$end
*/
// BEGIN_PROTOTYPE
template <class Base, class RecBase>
std::string CppAD::ADFun<Base,RecBase>::to_json(void)
// END_PROTOTYPE
{   using local::pod_vector;
    using local::opcode_t;
    // --------------------------------------------------------------------
    if( local::json::op_name2enum.size() == 0 )
    {   CPPAD_ASSERT_KNOWN( ! thread_alloc::in_parallel() ,
            "call to set_operator_info in parallel mode"
        );
        local::json::set_operator_info();
    }
    //
    // to_graph return values
    std::string                             function_name;
    size_t                                  n_dynamic_ind;
    size_t                                  n_independent;
    vector<std::string>                     atomic_name_vec;
    vector<double>                          constant_vec;
    vector<local::json::json_op_struct>     operator_vec;
    vector<size_t>                          operator_arg;
    vector<size_t>                          dependent_vec;
    //
    // graph corresponding to this function
    to_graph(
        function_name,
        n_dynamic_ind,
        n_independent,
        atomic_name_vec,
        constant_vec,
        operator_vec,
        operator_arg,
        dependent_vec
    );
    //
    // convert to json
    std::string graph;
    local::json::writer(
        graph,
        function_name,
        n_dynamic_ind,
        n_independent,
        atomic_name_vec,
        constant_vec,
        operator_vec,
        operator_arg,
        dependent_vec
    );
    //
    return graph;
}

# endif