# ! /bin/bash
# -----------------------------------------------------------------------------
# CppAD: C++ Algorithmic Differentiation: Copyright (C) 2003-06 Bradley M. Bell
#
# CppAD is distributed under multiple licenses. This distribution is under
# the terms of the 
#                     Common Public License Version 1.0.
#
# A copy of this license is included in the COPYING file of this distribution.
# Please visit http://www.coin-or.org/CppAD/ for information on other licenses.
# -----------------------------------------------------------------------------
#
# Define your subversion commit by editing the definition of 
# log_entry, add_list, change_list, delete_list and copy_branch below.
#
# log_entry
# The contents of this variable will be used as the log entry for the commit.
#
# add_list
# List of files that will be added to the repository during this commit.
# Do not use add_list to add directories; use the following instead:
# 	svn add --non-recursive dir
# 	svn commit directory -m "adding directory dir" dir
# where dir is replaced by the name of the directory. 
# Then use add_list to add the files within that directory.
#
# change_list
# List of files that are currently in repository and change during this commit.
#
# delete_list
# List of files that will be deleted from the repository during this commit.
#
# copy_branch
# You may specify up one other branch that should receive a copy of all that 
# changes that you are making with this commit. 
# This other branch cannot be the trunk and you must have a copy of it
# on your local machine. If copy_branch is empty; i.e., copy_branch="", 
# the changes will not be copied (and commited) into another branch.
#
# ----------------------------------------------------------------------
log_entry="template the vector type in DetByLu class.

svn_commit.sh: file that made this commit.
check_include_omh.sh: add speed/?.cpp files to check list.
whats_new_04.omh: remove cross references to deleted Adolc sections.
appendix.omh: move adolc speed sections below speed.omh.
config.h: update package version.
cppad/det_minor.cpp: change result to gradient (same as in compute_det_minor).
cppad/det_lu.cpp: change result to gradient (same as in compute_det_minor).

Make adolc speed test a modified copy of cppad speed test.
build.sh,
speed.omh,
adolc.omh,
adolc/det_minor.cpp,
speed_adolc.omh,
alloc_vec.cpp,
makefile.am,
adolc/det_lu.cpp.

Add vector type template parameter to constructor:
hes_lu_det.cpp,
jac_lu_det.cpp,
speed_cppad/det_lu.cpp,
det_by_minor.hpp,
?/det_lu.cpp,
det_by_lu.cpp,
det_by_lu.hpp.
"
add_list="
	omh/speed_adolc.omh
"
#
change_list="
	svn_commit.sh
	example/hes_lu_det.cpp
	example/jac_lu_det.cpp
	check_include_omh.sh
	speed_cppad/det_lu.cpp
	build.sh
	omh/speed.omh
	omh/whats_new_04.omh
	omh/whats_new_06.omh
	omh/appendix.omh
	cppad/config.h
	speed/det_by_minor.hpp
	speed/fadbad/det_lu.cpp
	speed/adolc/det_minor.cpp
	speed/adolc/makefile.am
	speed/adolc/det_lu.cpp
	speed/cppad/det_minor.cpp
	speed/cppad/det_lu.cpp
	speed/example/det_by_lu.cpp
	speed/det_by_lu.hpp
"
delete_list="
	omh/adolc.omh
	speed/adolc/alloc_mat.hpp
	speed/adolc/speed.cpp
	speed/adolc/alloc_vec.cpp
	speed/adolc/alloc_mat.cpp
	speed/adolc/example.cpp
	speed/adolc/alloc_vec.hpp
"
#
copy_branch="" 
# ----------------------------------------------------------------------
if [ "$copy_branch" != "" ]
then
	if [ ! -d ../branches/$copy_branch/.svn ]
	then
		echo "../branches/$copy_branch/.svn is not a directory"
	fi
fi
this_branch=`pwd | sed -e "s|.*/CppAD/||"`
echo "$this_branch: $log_entry" > svn_commit.log
count=0
for file in $add_list $change_list
do
	count=`expr $count + 1`
	ext=`echo $file | sed -e "s/.*\././"`
	if \
	[ -f $file            ] && \
	[ $ext  != ".sh"      ] && \
	[ $ext  != ".sed"     ] && \
	[ $ext  != ".vcproj"  ] && \
	[ $ext  != ".sln"     ] && \
	[ $ext  != ".vim"     ]
	then
		# automatic edits and backups
		echo "cp $file junk.$count"
		cp $file junk.$count
		sed -f svn_commit.sed < junk.$count > $file
		diff junk.$count $file
		if [ "$ext" == ".sh" ]
		then
			chmod +x $file
		fi
	fi
done
for file in $add_list
do
	echo "svn add $file ?"
	if [ "$copy_branch" != "" ]
	then
		echo "svn add ../branches/$copy_branch/$file ?"
	fi
done
for file in $change_list
do
	if [ "$copy_branch" != "" ]
	then
		echo "cp $file ../branches/$copy_branch/$file ?"
	fi
done
for file in $delete_list
do
	echo "svn delete $file ?"
	if [ "$copy_branch" != "" ]
	then
		echo "svn delete ../branches/$copy_branch/$file ?"
	fi
done
echo "-------------------------------------------------------------"
cat svn_commit.log
response=""
while [ "$response" != "c" ] && [ "$response" != "a" ]
do
	read -p "continue [c] or abort [a] ? " response
done
if [ "$response" = "a" ]
then
	echo "aborting commit"
	count=0
	for file in $add_list $change_list
	do
		count=`expr $count + 1`
		ext=`echo $file | sed -e "s/.*\././"`
		if \
		[ -f $file            ] && \
		[ $ext  != ".sh"      ] && \
		[ $ext  != ".sed"     ] && \
		[ $ext  != ".vcproj"  ] && \
		[ $ext  != ".sln"     ] && \
		[ $ext  != ".vim"     ]
		then
			# undo the automatic edits
			echo "mv junk.$count $file"
			mv junk.$count $file
			if [ "$ext" == ".sh" ]
			then
				chmod +x $file
			fi
		fi
	done
	exit 
fi
echo "continuing commit"
#
for file in $add_list
do
	svn add $file
done
for file in $delete_list
do
	svn delete $file
done
copy_list=""
if [ "$copy_branch" != "" ]
then
	for file in $add_list
	do
		target="../branches/$copy_branch/$file"
		cp $file $target
		svn add $target
		copy_list="$copy_list $target"
	done
	for file in $change_list
	do
		target="../branches/$copy_branch/$file"
		echo "cp $file $target"
		cp $file $target
		copy_list="$copy_list $target"
	done
	for file in $delete_list
	do
		svn delete $target
		target="../branches/$copy_branch/$file"
		copy_list="$copy_list $target"
	done
	
fi
svn commit --username bradbell --file svn_commit.log $add_list $change_list $delete_list $copy_list
