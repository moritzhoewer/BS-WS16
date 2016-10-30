#!/bin/bash
# Counting files
# Authors: Moritz Höwer, Jesko Treffler
# Date: 15.10.15

usage(){
cat <<EOF
usage:
$0 [-h | --help] This help text
$0 [ OPTIONS ] DIRECTORY

Print the number of files in DIRECTORY of types given in OPTIONS

Default: Print number of regular files in current directiory

OPTIONS :
File type options:
-f --regular-file  regular files
-l --symlink       symbolic links
-D --device        character and block devices
-d --directory     directories
-a --all           Include hidden files
Other options:
-e --echo          Print selected files
-v --verbose       print debugging messages

EOF
}
#-------------------------------------------------------------------
#                             main
if [ $# -lt 1 ]; then
	count=$(ls -l | grep ^- | wc -l)
	echo "There are $count files in this directory"
else
	# parse arguments
	for arg in "$@"
	do
		case $arg in
		"-h" | "--help")
			showHelp=1
		;;
		"-f" | "--regular-file")
			countFiles=1
		;;
		"-l" | "--symlink")
			countLinks=1
		;;
		"-D" | "--device")
			countDevices=1
		;;
		"-d" | "--directory")
			countDirs=1
		;;
		"-a" | "--all")
			showHidden=1
		;;
		"-v" | "--verbose")
			showVerbose=1
		;;
		"-e" | "--echo")
			echoFiles=1
		;;
		"--version")
			echo Version 1.0
			exit 0;
		;;
		-*)
			echo "Invalid Option $arg"
			usage
			exit -1
                ;;
		*)
			directory=$arg
		esac
	done

	# show help if flag was set
	if [ $showHelp ]; then
		usage
		exit 0
	fi

	# assemble command
	cmd="ls -l"

	# whether or not to show hidden files
	if [ $showHidden ]; then
	 cmd+="a"
	fi
	cmd+=" $directory"

	# continue with egrep
	cmd+=" | egrep \""

	# assemble the pattern for egrep starting with somehting that is
	# always false to make the following if statements easier
	egrepPattern="^k"

	# should files be counted
	if [ $countFiles ]; then
	 egrepPattern+="|^-"
	fi
	
	#should directories be counted
	if [ $countDirs ]; then
	 egrepPattern+="|^d"
	fi

	#should devices be counted
	if [ $countDevices ]; then
	 egrepPattern+="|^c|^b"
	fi
	
	#should links be counted
	if [ $countLinks ]; then
	 egrepPattern+="|^l"
	fi

	#finish egrep
	cmd+="$egrepPattern\""

	# continue with wordcount
	if [ $echoFiles ]; then
	 cmd+=" | tee >(wc -l) "
	else
	 cmd+=" | wc -l"
	fi

	# show command if verbose is on
	if [ $showVerbose ]; then
		echo "$cmd"
	fi

	# execute comand and display result
	eval $cmd
	#echo "There are $count in directory"
fi

exit 0
