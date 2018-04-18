
# ifndef CPPAD_LOCAL_PLAYER_HPP
# define CPPAD_LOCAL_PLAYER_HPP

/* --------------------------------------------------------------------------
CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-18 Bradley M. Bell

CppAD is distributed under multiple licenses. This distribution is under
the terms of the
                    Eclipse Public License Version 1.0.

A copy of this license is included in the COPYING file of this distribution.
Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
-------------------------------------------------------------------------- */

# include <cppad/local/user_state.hpp>
# include <cppad/local/is_pod.hpp>

namespace CppAD { namespace local { // BEGIN_CPPAD_LOCAL_NAMESPACE
/*!
\file player.hpp
File used to define the player and player_const_iterator classes.
*/

/*!
Class used to store and play back an operation sequence recording.

\tparam Base
These were AD< Base > operations when recorded. Operations during playback
are done using the type Base .
*/

template <class Base> class player_const_iterator;
template <class Base> class player_const_random_iterator;

template <class Base>
class player {
private:
	// ----------------------------------------------------------------------
	// information that defines the recording

	/// Number of variables in the recording.
	size_t num_var_rec_;

	/// number of vecad load opeations in the reconding
	size_t num_load_op_rec_;

	/// Number of VecAD vectors in the recording
	size_t num_vecad_vec_rec_;

	/// The operators in the recording.
	pod_vector<OpCode> op_vec_;

	/// The operation argument indices in the recording
	pod_vector<addr_t> arg_vec_;

	/// The parameters in the recording.
	/// Note that Base may not be plain old data, so use false in consructor.
	pod_vector<Base> par_vec_;

	/// Character strings ('\\0' terminated) in the recording.
	pod_vector<char> text_vec_;

	/// The VecAD indices in the recording.
	pod_vector<addr_t> vecad_ind_vec_;

	// ----------------------------------------------------------------------
	// Redundant information that simplifies and speeds up iterating through
	// the operation sequence.

	/// index in arg_vec_ corresonding to the first argument for each operator
	pod_vector<addr_t> op2arg_vec_;

	/*!
	Index of the result variable for each operator. If the operator has
	no results, this is not defined. The invalid index num_var_rec_ is used
	when NDEBUG is not defined. If the operator has more than one result, this
	is the primary result; i.e., the last result. Auxillary are only used by
	the operator and not used by other operators.
	*/
	pod_vector<addr_t> op2var_vec_;

	/// Mapping from primary variable index to corresponding operator index.
	/// This is used to traverse sub-graphs of the operation sequence.
	/// This value is valid (invalid) for primary (auxillary) variables.
	pod_vector<addr_t> var2op_vec_;

public:
	// =================================================================
	/// constructor
	player(void) :
	num_var_rec_(0)      ,
	num_load_op_rec_(0)
	{ }

	// =================================================================
	/// destructor
	~player(void)
	{ }

	// ===============================================================
	/*!
	Moving an operation sequence from a recorder to this player

	\param rec
	the object that was used to record the operation sequence. After this
	operation, the state of the recording is no longer defined. For example,
	the pod_vector member variables in this have been swapped with rec.

	\param n_ind
	the number of independent variables (only used for error checking
	when NDEBUG is not defined).

	\par
	Use an assert to check that the length of the following vectors is
	less than the maximum possible value for addr_t; i.e., that an index
	in these vectors can be represented using the type addr_t:
	op_vec_, vecad_ind_vec_, arg_vec_, par_vec_, text_vec_.
	*/
	void get(recorder<Base>& rec, size_t n_ind)
	{
# ifndef NDEBUG
		size_t addr_t_max = size_t( std::numeric_limits<addr_t>::max() );
# endif
		// just set size_t values
		num_var_rec_        = rec.num_var_rec_;
		num_load_op_rec_    = rec.num_load_op_rec_;

		// op_vec_
		op_vec_.swap(rec.op_vec_);
		CPPAD_ASSERT_UNKNOWN(op_vec_.size() < addr_t_max );

		// op_arg_vec_
		arg_vec_.swap(rec.arg_vec_);
		CPPAD_ASSERT_UNKNOWN(arg_vec_.size()    < addr_t_max );

		// par_vec_
		par_vec_.swap(rec.par_vec_);
		CPPAD_ASSERT_UNKNOWN(par_vec_.size() < addr_t_max );

		// text_rec_
		text_vec_.swap(rec.text_vec_);
		CPPAD_ASSERT_UNKNOWN(text_vec_.size() < addr_t_max );

		// vecad_ind_vec_
		vecad_ind_vec_.swap(rec.vecad_ind_vec_);
		CPPAD_ASSERT_UNKNOWN(vecad_ind_vec_.size() < addr_t_max );

		// num_vecad_vec_rec_
		num_vecad_vec_rec_ = 0;
		{	// vecad_ind_vec_ contains size of each VecAD followed by
			// the parameter indices used to inialize it.
			size_t i = 0;
			while( i < vecad_ind_vec_.size() )
			{	num_vecad_vec_rec_++;
				i += vecad_ind_vec_[i] + 1;
			}
			CPPAD_ASSERT_UNKNOWN( i == vecad_ind_vec_.size() );
		}

		// op2arg_vec_, op2var_vec_, var2op_vec_
		CPPAD_ASSERT_UNKNOWN( op_vec_[0] == BeginOp );
		CPPAD_ASSERT_NARG_NRES(BeginOp, 1, 1);
		addr_t  num_op    = addr_t( op_vec_.size() );
		addr_t  var_index = 0;
		addr_t  arg_index = 0;
		//
		op2arg_vec_.resize( num_op );
		op2var_vec_.resize( num_op );
		var2op_vec_.resize( num_var_rec_ );
# ifndef NDEBUG
		// value of var2op for auxillary variables is num_op (invalid)
		for(size_t i_var = 0; i_var < num_var_rec_; ++i_var)
			var2op_vec_[i_var] = num_op;
		// value of op2var is num_var (invalid) when NumRes(op) = 0
		for(addr_t i_op = 0; i_op < num_op; ++i_op)
			op2var_vec_[i_op] = addr_t( num_var_rec_ );
# endif
		for(addr_t i_op = 0; i_op < num_op; ++i_op)
		{	OpCode  op          = op_vec_[i_op];
			//
			// index of first argument for this operator
			op2arg_vec_[i_op]   = arg_index;
			arg_index          += addr_t( NumArg(op) );
			//
			// index of first result for next operator
			var_index  += addr_t( NumRes(op) );
			if( NumRes(op) > 0 )
			{	// index of last (primary) result for this operator
				op2var_vec_[i_op] = var_index - 1;
				//
				// mapping from primary variable to its operator
				var2op_vec_[var_index - 1] = i_op;
			}
			// CSumOp
			if( op == CSumOp )
			{	CPPAD_ASSERT_UNKNOWN( NumArg(CSumOp) == 0 );
				//
				// pointer to first argument for this operator
				addr_t* op_arg = arg_vec_.data() + arg_index;
				//
				// The actual number of arugments for this operator is
				// op_arg[0] + op_arg[1] + 4
				// Correct index of first argument for next operator
				arg_index += op_arg[0] + op_arg[1] + 4;
			}
			//
			// CSkip
			if( op == CSkipOp )
			{	CPPAD_ASSERT_UNKNOWN( NumArg(CSumOp) == 0 );
				//
				// pointer to first argument for this operator
				addr_t* op_arg = arg_vec_.data() + arg_index;
				//
				// The actual number of arugments for this operator is
				// 7 + op_arg[4] + op_arg[5].
				// Correct index of first argument for next operator.
				arg_index += 7 + op_arg[4] + op_arg[5];
			}
		}
		check_inv_op(n_ind);
		check_dag();
	}
	// ----------------------------------------------------------------------
	/*!
	Check that InvOp operators start with second operator and are contiguous,
	and there are n_ind of them.
	*/
# ifdef NDEBUG
	void check_inv_op(size_t n_ind)
	{	return; }
# else
	void check_inv_op(size_t n_ind)
	{	size_t num_op = op_vec_.size();
		CPPAD_ASSERT_UNKNOWN( op_vec_[0] = BeginOp );
		CPPAD_ASSERT_UNKNOWN( op_vec_[1] = InvOp   );
		// start at second operator
		for(size_t i_op = 1; i_op < num_op; ++i_op)
		{	OpCode op = op_vec_[i_op];
			CPPAD_ASSERT_UNKNOWN( (op == InvOp) == (i_op <= n_ind) );
		}
		return;
	}

# endif
	// ----------------------------------------------------------------------
	/*!
	Check arguments that are variables, to make sure the have value less
	than or equal to the previously created variable. This is the directed
	acyclic graph condition (DAG).
	*/
# ifdef NDEBUG
	void check_dag(void)
	{	return; }
# else
	void check_dag(void)
	{
		size_t  num_op       = op_vec_.size();
		addr_t arg_var_bound = 0;
		for(size_t i = 0; i < num_op; i++)
		{	OpCode op = op_vec_[i];
			addr_t* op_arg = arg_vec_.data() + op2arg_vec_[i];
			switch(op)
			{
				// cases where nothing to do
				case BeginOp:
				case EndOp:
				case InvOp:
				case LdpOp:
				case ParOp:
				case UserOp:
				case UsrapOp:
				case UsrrpOp:
				case UsrrvOp:
				case StppOp:
				break;

				// only first argument is a variable
				case AbsOp:
				case AcosOp:
				case AcoshOp:
				case AsinOp:
				case AsinhOp:
				case AtanOp:
				case AtanhOp:
				case CosOp:
				case CoshOp:
				case DivvpOp:
				case ErfOp:
				case ExpOp:
				case Expm1Op:
				case LevpOp:
				case LogOp:
				case Log1pOp:
				case LtvpOp:
				case PowvpOp:
				case SignOp:
				case SinOp:
				case SinhOp:
				case SqrtOp:
				case SubvpOp:
				case TanOp:
				case TanhOp:
				case UsravOp:
				case ZmulvpOp:
				CPPAD_ASSERT_UNKNOWN(op_arg[0] <= arg_var_bound );
				break;

				// only second argument is a variable
				case AddpvOp:
				case DisOp:
				case DivpvOp:
				case EqpvOp:
				case LdvOp:
				case LepvOp:
				case LtpvOp:
				case MulpvOp:
				case NepvOp:
				case PowpvOp:
				case StvpOp:
				case SubpvOp:
				case ZmulpvOp:
				CPPAD_ASSERT_UNKNOWN(op_arg[1] <= arg_var_bound );
				break;

				// only first and second arguments are variables
				case AddvvOp:
				case DivvvOp:
				case EqvvOp:
				case LevvOp:
				case LtvvOp:
				case MulvvOp:
				case NevvOp:
				case PowvvOp:
				case SubvvOp:
				case ZmulvvOp:
				CPPAD_ASSERT_UNKNOWN(op_arg[0] <= arg_var_bound );
				CPPAD_ASSERT_UNKNOWN(op_arg[1] <= arg_var_bound );
				break;

				// StpvOp
				case StpvOp:
				CPPAD_ASSERT_UNKNOWN(op_arg[2] <= arg_var_bound );
				break;

				// StvvOp
				case StvvOp:
				CPPAD_ASSERT_UNKNOWN(op_arg[1] <= arg_var_bound );
				CPPAD_ASSERT_UNKNOWN(op_arg[2] <= arg_var_bound );
				break;

				// CSumOp
				case CSumOp:
				{	addr_t num_add = op_arg[0];
					addr_t num_sub = op_arg[1];
					for(addr_t j = 0; j < num_add + num_sub; j++)
						CPPAD_ASSERT_UNKNOWN(op_arg[3+j] <= arg_var_bound);
				}
				break;

				// CExpOp
				case CExpOp:
				if( op_arg[1] & 1 )
					CPPAD_ASSERT_UNKNOWN( op_arg[2] <= arg_var_bound);
				if( op_arg[1] & 2 )
					CPPAD_ASSERT_UNKNOWN( op_arg[3] <= arg_var_bound);
				if( op_arg[1] & 4 )
					CPPAD_ASSERT_UNKNOWN( op_arg[4] <= arg_var_bound);
				if( op_arg[1] & 8 )
					CPPAD_ASSERT_UNKNOWN( op_arg[5] <= arg_var_bound);
				break;

				// PriOp
				case PriOp:
				if( op_arg[0] & 1 )
					CPPAD_ASSERT_UNKNOWN( op_arg[1] <= arg_var_bound);
				if( op_arg[0] & 2 )
					CPPAD_ASSERT_UNKNOWN( op_arg[3] <= arg_var_bound);
				break;

				// CSkipOp, PriOp
				case CSkipOp:
				if( op_arg[1] & 1 )
					CPPAD_ASSERT_UNKNOWN( op_arg[2] <= arg_var_bound);
				if( op_arg[1] & 2 )
					CPPAD_ASSERT_UNKNOWN( op_arg[3] <= arg_var_bound);
				break;

				default:
				CPPAD_ASSERT_UNKNOWN(false);
				break;


			}
			if( NumRes(op) > 0 )
			{	addr_t var_index = op2var_vec_[i];
				if( var_index > 0 )
				{	CPPAD_ASSERT_UNKNOWN(arg_var_bound < var_index);
				}
				else
				{	CPPAD_ASSERT_UNKNOWN(arg_var_bound == var_index);
				}
				//
				arg_var_bound = var_index;
			}
		}
	}
# endif
	// ===============================================================
	/*!
	Copying an operation sequence from another player to this one

	\param play
	the object that contains the operatoion sequence to copy.
	*/
	void operator=(const player& play)
	{
		num_var_rec_        = play.num_var_rec_;
		num_load_op_rec_    = play.num_load_op_rec_;
		op_vec_             = play.op_vec_;
		num_vecad_vec_rec_  = play.num_vecad_vec_rec_;
		vecad_ind_vec_      = play.vecad_ind_vec_;
		arg_vec_            = play.arg_vec_;
		par_vec_            = play.par_vec_;
		text_vec_           = play.text_vec_;
		op2arg_vec_         = play.op2arg_vec_;
		op2var_vec_         = play.op2var_vec_;
		var2op_vec_         = play.var2op_vec_;
	}
	// ===============================================================
	/// Erase the recording stored in the player
	void Erase(void)
	{
		num_var_rec_       = 0;
		num_load_op_rec_   = 0;
		num_vecad_vec_rec_ = 0;

		op_vec_.resize(0);
		vecad_ind_vec_.resize(0);
		arg_vec_.resize(0);
		par_vec_.resize(0);
		text_vec_.resize(0);
		op2arg_vec_.resize(0);
		op2var_vec_.resize(0);
		var2op_vec_.resize(0);
	}
	// ================================================================
	// const functions that retrieve infromation from this player
	// ================================================================
	/*!
	\brief
	fetch the operator corresponding to a primary variable

	\param var_index
	must be the index of a primary variable.

	\return
	is the index of the operator corresponding to this primary variable.
	*/
	size_t var2op(size_t var_index) const
	{	size_t i_op = var2op_vec_[var_index];
		// check that var_index is a primary variable index
		CPPAD_ASSERT_UNKNOWN( i_op < op_vec_.size() );
		return i_op;
	}
	/*!
	\brief
	fetch the information corresponding to an operator

	\param op_index
	index for this operator [in]

	\param op [out]
	op code for this operator.

	\param op_arg [out]
	pointer to the first arguement to this operator.

	\param var_index [out]
	index of the last variable (primary variable) for this operator.
	If there is no primary variable for this operator, i_var not sepcified
	and could have any value.
	*/
	void get_op_info(
		size_t         op_index   ,
		OpCode&        op         ,
		const addr_t*& op_arg     ,
		size_t&        var_index  ) const
	{	op        = OpCode( op_vec_[op_index] );
		op_arg    = op2arg_vec_[op_index] + arg_vec_.data();
		var_index = op2var_vec_[op_index];
		return;
	}
	/*!
	\brief
	unpack extra information corresponding to a UserOp

	\param op [in]
	must be a UserOp

	\param op_arg [in]
	is the arguments for this operator

	\param user_old [out]
	is the extra information passed to the old style user atomic functions.

	\param user_m   [out]
	is the number of results for this user atmoic function.

	\param user_n   [out]
	is the number of arguments for this user atmoic function.

	\return
	Is a pointer to this user atomic function.
	*/
	atomic_base<Base>* get_user_info(
		const OpCode     op         ,
		const addr_t*    op_arg     ,
		size_t&          user_old   ,
		size_t&          user_m     ,
		size_t&          user_n     ) const
	{	atomic_base<Base>* user_atom;
		//
		CPPAD_ASSERT_UNKNOWN( op == UserOp );
		CPPAD_ASSERT_NARG_NRES(op, 4, 0);
		//
		user_old = op_arg[1];
		user_n   = op_arg[2];
		user_m   = op_arg[3];
		CPPAD_ASSERT_UNKNOWN( user_n > 0 );
		//
		size_t user_index = size_t( op_arg[0] );
		user_atom = atomic_base<Base>::class_object(user_index);
# ifndef NDEBUG
		if( user_atom == CPPAD_NULL )
		{	// user_atom is null so cannot use user_atom->afun_name()
			std::string msg = atomic_base<Base>::class_name(user_index)
				+ ": atomic_base function has been deleted";
			CPPAD_ASSERT_KNOWN(false, msg.c_str() );
		}
# endif
		// the atomic_base object corresponding to this user function
		user_atom = atomic_base<Base>::class_object(user_index);
		CPPAD_ASSERT_UNKNOWN( user_atom != CPPAD_NULL );
		return user_atom;
	}
	/*!
	\brief
	fetch an operator from the recording.

	\return
	the i-th operator in the recording.

	\param i
	the index of the operator in recording
	*/
	OpCode GetOp (size_t i) const
	{	return OpCode(op_vec_[i]); }

	/*!
	\brief
	Fetch a VecAD index from the recording.

	\return
	the i-th VecAD index in the recording.

	\param i
	the index of the VecAD index in recording
	*/
	size_t GetVecInd (size_t i) const
	{	return vecad_ind_vec_[i]; }

	/*!
	\brief
	Fetch a parameter from the recording.

	\return
	the i-th parameter in the recording.

	\param i
	the index of the parameter in recording
	*/
	Base GetPar(size_t i) const
	{	return par_vec_[i]; }

	/*!
	\brief
	Fetch entire parameter vector from the recording.

	\return
	the entire parameter vector.

	*/
	const Base* GetPar(void) const
	{	return par_vec_.data(); }

	/*!
	\brief
	Fetch a '\\0' terminated string from the recording.

	\return
	the beginning of the string.

	\param i
	the index where the string begins.
	*/
	const char *GetTxt(size_t i) const
	{	CPPAD_ASSERT_UNKNOWN(i < text_vec_.size() );
		return text_vec_.data() + i;
	}

	/// Fetch number of variables in the recording.
	size_t num_var_rec(void) const
	{	return num_var_rec_; }

	/// Fetch number of vecad load operations
	size_t num_load_op_rec(void) const
	{	return num_load_op_rec_; }

	/// Fetch number of operators in the recording.
	size_t num_op_rec(void) const
	{	return op_vec_.size(); }

	/// Fetch number of VecAD indices in the recording.
	size_t num_vec_ind_rec(void) const
	{	return vecad_ind_vec_.size(); }

	/// Fetch number of VecAD vectors in the recording
	size_t num_vecad_vec_rec(void) const
	{	return num_vecad_vec_rec_; }

	/// Fetch number of argument indices in the recording.
	size_t num_op_arg_rec(void) const
	{	return arg_vec_.size(); }

	/// Fetch number of parameters in the recording.
	size_t num_par_rec(void) const
	{	return par_vec_.size(); }

	/// Fetch number of characters (representing strings) in the recording.
	size_t num_text_rec(void) const
	{	return text_vec_.size(); }

	/// Fetch a rough measure of amount of memory used to store recording
	/// using just lengths, not capacities; see the heading size_op_arg
	/// in the file seq_property.omh.
	size_t Memory(void) const
	{	// check assumptions made by ad_fun<Base>::size_op_seq()
		CPPAD_ASSERT_UNKNOWN( op_vec_.size() == num_op_rec() );
		CPPAD_ASSERT_UNKNOWN( op2arg_vec_.size() == num_op_rec() );
		CPPAD_ASSERT_UNKNOWN( op2var_vec_.size() == num_op_rec() );
		CPPAD_ASSERT_UNKNOWN( arg_vec_.size()    == num_op_arg_rec() );
		CPPAD_ASSERT_UNKNOWN( par_vec_.size() == num_par_rec() );
		CPPAD_ASSERT_UNKNOWN( text_vec_.size() == num_text_rec() );
		CPPAD_ASSERT_UNKNOWN( vecad_ind_vec_.size() == num_vec_ind_rec() );
		//
		return op_vec_.size()        * sizeof(OpCode)
		     + arg_vec_.size()       * sizeof(addr_t)
		     + par_vec_.size()       * sizeof(Base)
		     + text_vec_.size()      * sizeof(char)
		     + vecad_ind_vec_.size() * sizeof(addr_t)
		     + op_vec_.size()        * sizeof(addr_t) * 3
		;
	}
	typedef player_const_iterator<Base> const_iterator;
	/// begin
	const_iterator begin(void) const
	{	size_t op_index = 0;
		size_t num_var  = num_var_rec_;
		return const_iterator(num_var, &op_vec_, &arg_vec_, op_index);
	}
	/// end
	const_iterator end(void) const
	{	size_t op_index = op_vec_.size();
		size_t num_var  = num_var_rec_;
		return const_iterator(num_var, &op_vec_, &arg_vec_, op_index);
	}
	typedef player_const_random_iterator<Base> const_random_iterator;
	/// random
	const_random_iterator random_iterator(void) const
	{	return const_random_iterator(
			&op_vec_, &arg_vec_, &op2arg_vec_, &op2var_vec_, &var2op_vec_
		);
	}

};

// ============================================================================
/// const_iterator for a player object.
template <class Base>
class player_const_iterator {
private:
	/// number of variables in tape
	const size_t num_var_;

	/// op_vec_
	const pod_vector<OpCode>* op_vec_;

	/// arg_vec_
	const pod_vector<addr_t>* arg_vec_;

	/// index of current operator
	size_t op_index_;

	/// index of first argument for current operator
	size_t arg_index_;

	/// index of first result for current operator
	size_t var_index_;
public:
	/// Create a iterator starting either at beginning or end of tape
	player_const_iterator(
		/// number of variables in tape
		size_t                    num_var    ,
		/// operators in this player
		const pod_vector<OpCode>* op_vec     ,
		/// operator arguments for this player
		const pod_vector<addr_t>* arg_vec    ,
		/// operator index to start iterator at
		/// must be 0 for begin() and op_vec->size() for end()
		size_t                    op_index   )
	:
	num_var_  ( num_var ),
	op_vec_   ( op_vec ),
	arg_vec_  ( arg_vec )
	{	if( op_index == 0 )
		{	op_index_  = 0;
			arg_index_ = 0;
			var_index_ = 0;
		}
		else
		{	CPPAD_ASSERT_UNKNOWN(op_index == op_vec->size());
			op_index_  = op_vec->size();
			arg_index_ = arg_vec->size();
			var_index_ = num_var;
		}
	}
	/*!
	Advance iterator to next operator
	*/
	player_const_iterator<Base>& operator++(void)
	{
		OpCode op   = (*op_vec_)[op_index_];
		op_index_  += 1;
		arg_index_ += NumArg(op);
		var_index_ += NumRes(op);
		bool done   = (op != CSumOp) & (op != CSkipOp);
		if( done )
			return *this;
		//
		// number of arguments for this operator depends on argument data
		CPPAD_ASSERT_UNKNOWN( NumArg(op) == 0 );
		const addr_t* arg = arg_vec_->data() + arg_index_;
		//
		// CSumOp
		if( op == CSumOp )
		{	//
			addr_t n_var      = arg[0] + arg[1];
			CPPAD_ASSERT_UNKNOWN( n_var == arg[3 + n_var] );
			//
			// add actual number of arguments to arg_index_
			arg_index_ += 4 + n_var;
			//
			return *this;
		}
		//
		// CSkip
		if ( op == CSkipOp )
		{	//
			addr_t n_skip     = arg[4] + arg[5];
			CPPAD_ASSERT_UNKNOWN( n_skip == arg[6 + n_skip] );
			//
			// add actual number of arguments to arg_index_
			arg_index_ += 7 + n_skip;
			//
			return *this;
		}
		CPPAD_ASSERT_UNKNOWN( false );
		return *this;
	}
	/*!
	Backup iterator to previous operator
	*/
	player_const_iterator<Base>& operator--(void)
	{	//
		CPPAD_ASSERT_UNKNOWN( 1 <= op_index_ );
		op_index_  -= 1;
		OpCode op   = (*op_vec_)[op_index_];
		//
		CPPAD_ASSERT_UNKNOWN( NumArg(op) <= arg_index_ );
		arg_index_ -= NumArg(op);
		//
		CPPAD_ASSERT_UNKNOWN( NumRes(op) <= var_index_ );
		var_index_ -= NumRes(op);
		//
		bool done   = (op != CSumOp) & (op != CSkipOp);
		if( done )
			return *this;
		//
		// CSumOp
		if( op == CSumOp )
		{	// number of arguments for this operator depends on argument data
			CPPAD_ASSERT_UNKNOWN( NumArg(CSumOp) == 0 );
			//
			// number of variables is stored in last argument
			CPPAD_ASSERT_UNKNOWN( 1 < arg_index_ );
			addr_t n_var = (*arg_vec_)[arg_index_ - 1];
			//
			// index of first argument to this operator
			arg_index_ -= 4 + n_var;
# ifndef NDEBUG
			const addr_t* arg = arg_vec_->data() + arg_index_;
			CPPAD_ASSERT_UNKNOWN( arg[0] + arg[1] == n_var );
# endif
			return *this;
		}
		//
		// CSkip
		if( op == CSkipOp )
		{	// number of arguments for this operator depends on argument data
			CPPAD_ASSERT_UNKNOWN( NumArg(CSumOp) == 0 );
			//
			// number to possibly skip is stored in last argument
			CPPAD_ASSERT_UNKNOWN( 1 < arg_index_ );
			addr_t n_skip = (*arg_vec_)[arg_index_ - 1];
			//
			// index of frist argument to this operator
			arg_index_ -= 7 + n_skip;
# ifndef NDEBUG
			const addr_t* arg = arg_vec_->data() + arg_index_;
			CPPAD_ASSERT_UNKNOWN( arg[4] + arg[5] == n_skip );
# endif
			return *this;
		}
		CPPAD_ASSERT_UNKNOWN( false );
		return *this;
	}
	/*!
	\brief
	Get information corresponding to current operator.

	\param op [out]
	op code for this operator.

	\param op_arg [out]
	pointer to the first arguement to this operator.

	\param var_index [out]
	index of the last variable (primary variable) for this operator.
	If there is no primary variable for this operator, var_index
	is not sepcified and could have any value.
	*/
	void op_info(
		OpCode&        op         ,
		const addr_t*& op_arg     ,
		size_t&        var_index  ) const
	{	// op
		op        = (*op_vec_)[op_index_];
		//
		// op_arg
		CPPAD_ASSERT_UNKNOWN( arg_index_ + NumArg(op) <= arg_vec_->size() );
		op_arg    = arg_vec_->data() + arg_index_;
		//
		// var_index
		CPPAD_ASSERT_UNKNOWN( var_index_ + NumRes(op) <= num_var_ );
		var_index = var_index_ + NumRes(op) - 1;
	}
	/// current operator index
	size_t op_index(void)
	{	return op_index_; }
	/*!
	\brief
	Unpack extra information when current op is a UserOp

	\param user_old [out]
	is the extra information passed to the old style user atomic functions.

	\param user_m [out]
	is the number of results for this user atmoic function.

	\param user_n [out]
	is the number of arguments for this user atmoic function.

	\return
	is a pointer to this user atomic function.
	*/
	atomic_base<Base>* user_info(
		size_t&          user_old   ,
		size_t&          user_m     ,
		size_t&          user_n     ) const
	{	atomic_base<Base>* user_atom;
		//
		// get operator infromation
		OpCode op;
		const addr_t* op_arg;
		size_t var_index;
		op_info(op, op_arg, var_index);
		//
		CPPAD_ASSERT_UNKNOWN( op == UserOp );
		CPPAD_ASSERT_NARG_NRES(op, 4, 0);
		//
		// return UserOp info
		user_old = op_arg[1];
		user_n   = op_arg[2];
		user_m   = op_arg[3];
		CPPAD_ASSERT_UNKNOWN( user_n > 0 );
		//
		size_t user_index = size_t( op_arg[0] );
		user_atom = atomic_base<Base>::class_object(user_index);
# ifndef NDEBUG
		if( user_atom == CPPAD_NULL )
		{	// user_atom is null so cannot use user_atom->afun_name()
			std::string msg = atomic_base<Base>::class_name(user_index)
				+ ": atomic_base function has been deleted";
			CPPAD_ASSERT_KNOWN(false, msg.c_str() );
		}
# endif
		//
		return user_atom;
	}
};
// ============================================================================
/// This class provides both random accesss and subgraph accesss to a player.
/// The subgraph access functions ++, --, op_info, and user_inf are designed
/// to work like corresponding functions in player_const_iterator class.
template <class Base>
class player_const_random_iterator {
private:
	/// mapping from operator index to operator
	const pod_vector<OpCode>* const op_vec_;

	/// mapping from argument index to argument value
	const pod_vector<addr_t>* const arg_vec_;

	/// mapping from operator index to argument index
	const pod_vector<addr_t>* const op2arg_vec_;

	/// mapping from operator index to primary variable index
	const pod_vector<addr_t>* const op2var_vec_;

	/// mapping from primary variable index to operator index
	const pod_vector<addr_t>* const var2op_vec_;

	/// mapping from subgraph index to operator index
	const pod_vector<addr_t>* subgraph_;

	/// index of the current operator in the subgraph
	size_t subgraph_index_;

public:
	/*!
	Set random access information corresponding to a player.
	This goes through the operation sequence and makes a new
	mapping for op2arg_vec_, op2var_vec_, and var2op_vec_.
	It also clears any subgraph information.
	*/
	player_const_random_iterator(
		/// operators in this player
		const pod_vector<OpCode>* op_vec     ,
		/// operator arguments for this player
		const pod_vector<addr_t>* arg_vec    ,
		/// mapping from operator to argument
		const pod_vector<addr_t>* op2arg_vec ,
		/// mapping from operator to variable
		const pod_vector<addr_t>* op2var_vec ,
		/// mapping from variable to operator
		const pod_vector<addr_t>* var2op_vec )
	:
	arg_vec_        (arg_vec_)    ,
	op2arg_vec_     (op2arg_vec)  ,
	op2var_vec_     (op2var_vec)  ,
	var2op_vec_     (var2op_vec)  ,
	subgraph_       (CPPAD_NULL)  ,
	subgraph_index_ (0)
	{ }
	/// num_op
	size_t num_op(void) const
	{	return op_vec_->size();
	}
	/// num_var
	size_t num_var(void) const
	{	return var2op_vec_->size();
	}
	/// get_op
	OpCode get_op(size_t op_index) const
	{	return (*op_vec_)[op_index];
	}
	/// var2op
	size_t var2op(size_t var_index) const
	{	return (*var2op_vec_)[var_index];
	}
	/// Set the subgraph for ++ and -- to iterator over
	void set_subgraph(
		/// pointer to map from subgraph index to operator index
		const pod_vector<addr_t>* subgraph       ,
		/// subgraph index to start ++ and  -- operators at
		size_t                    subgraph_index )
	{	subgraph_       = subgraph;
		subgraph_index_ = subgraph_index;
	}
	/// advance to next operator in the subgraph
	player_const_random_iterator& operator++(void)
	{	CPPAD_ASSERT_UNKNOWN( subgraph_ != CPPAD_NULL );
		CPPAD_ASSERT_UNKNOWN( subgraph_index_ + 1 < subgraph_->size() );
		--subgraph_index_;
	}
	/// backup to previous operator in the subgraph
	player_const_random_iterator& operator--(void)
	{	CPPAD_ASSERT_UNKNOWN( subgraph_ != CPPAD_NULL );
		CPPAD_ASSERT_UNKNOWN( 0 < subgraph_index_ );
		--subgraph_index_;
	}
	/// operator index for current operator in the subgraph
	size_t op_index(void) const
	{	return (*subgraph_)[subgraph_index_]; }
	/*!
	\brief
	random access of information any operator in the graph.

	\param op_index
	index for this operator [in]

	\param op [out]
	op code for this operator.

	\param op_arg [out]
	pointer to the first arguement to this operator.

	\param var_index [out]
	index of the last variable (primary variable) for this operator.
	If there is no primary variable for this operator, i_var not sepcified
	and could have any value.
	*/
	void op_info(
		size_t         op_index   ,
		OpCode&        op         ,
		const addr_t*& op_arg     ,
		size_t&        var_index  ) const
	{	// op
		op              = (*op_vec_)[op_index];
		//
		// op_arg
		size_t arg_index = (*op2arg_vec_)[op_index];
		CPPAD_ASSERT_UNKNOWN( arg_index + NumArg(op) <= arg_vec_->size() );
		op_arg = arg_vec_->data() + arg_index;
		//
		// var_index
		var_index = (*op2var_vec_)[op_index];
		CPPAD_ASSERT_UNKNOWN(
			var_index < var2op_vec_->size() ||
			( NumRes(op) == 0 && var_index == var2op_vec_->size() )
		);
		return;
	}
	/*!
	\brief
	Get information corresponding to current operator in the subgraph.

	\param op [out]
	op code for this operator.

	\param op_arg [out]
	pointer to the first arguement for this operator.

	\param var_index [out]
	index of the last variable (primary variable) for this operator.
	If there is no primary variable for this operator, var_index
	is not sepcified and could have any value.
	*/
	void op_info(
		OpCode&        op         ,
		const addr_t*& op_arg     ,
		size_t&        var_index  ) const
	{	// op
		size_t op_index = (*subgraph_)[subgraph_index_];
		op_info(op_index, op, op_arg, var_index);
	}
	/*!
	\brief
	random access of extra information when current operator is UserOp.

	\param op_index
	index for this operator [in]

	\param user_old [out]
	is the extra information passed to the old style user atomic functions.

	\param user_m [out]
	is the number of results for this user atmoic function.

	\param user_n [out]
	is the number of arguments for this user atmoic function.

	\return
	is a pointer to this user atomic function.
	*/
	atomic_base<Base>* user_info(
		size_t           op_index   ,
		size_t&          user_old   ,
		size_t&          user_m     ,
		size_t&          user_n     ) const
	{	// get operator infromation
		OpCode op;
		const addr_t* op_arg;
		size_t var_index;
		op_info(op_index, op, op_arg, var_index);
		//
		CPPAD_ASSERT_UNKNOWN( op == UserOp );
		CPPAD_ASSERT_NARG_NRES(op, 4, 0);
		//
		// return UserOp info
		user_old = op_arg[1];
		user_n   = op_arg[2];
		user_m   = op_arg[3];
		CPPAD_ASSERT_UNKNOWN( user_n > 0 );
		//
		size_t user_index = size_t( op_arg[0] );
		atomic_base<Base>* user_atom =
			atomic_base<Base>::class_object(user_index);
# ifndef NDEBUG
		if( user_atom == CPPAD_NULL )
		{	// user_atom is null so cannot use user_atom->afun_name()
			std::string msg = atomic_base<Base>::class_name(user_index)
				+ ": atomic_base function has been deleted";
			CPPAD_ASSERT_KNOWN(false, msg.c_str() );
		}
# endif
		//
		return user_atom;
	}
	/*!
	\brief
	Get extra information when current operator in the subgraph is UserOp.

	\param user_old [out]
	is the extra information passed to the old style user atomic functions.

	\param user_m [out]
	is the number of results for this user atmoic function.

	\param user_n [out]
	is the number of arguments for this user atmoic function.

	\return
	is a pointer to this user atomic function.
	*/
	atomic_base<Base>* user_info(
		size_t&          user_old   ,
		size_t&          user_m     ,
		size_t&          user_n     ) const
	{	// get operator infromation
		size_t op_index = (*subgraph_)[subgraph_index_];
		return user_info(op_index, user_old, user_m, user_n);
	}
};

} } // END_CPPAD_lOCAL_NAMESPACE
# endif
