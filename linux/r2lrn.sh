#!/bin/bash
# (C) Kirils Solovjovs, 2019
# written in whatever licence r2 is written in

set -e

function cleanup {
  [ -f _console ] && rm _console
  [ -z $(jobs -p) ] || kill $(jobs -p)
}

trap cleanup EXIT

OLDDIR="$(pwd)"
cd "$(dirname "$0")"
APPDIR="$(pwd)"
LEVELDIR="$APPDIR/levels"

cd "$OLDDIR"

function show_hint {
	[ -z "$hint" ] && echo "Sorry. Level developer was a meany and didn't include a hint." && return 0
	echo "Hint: $hint"

}

function take_answer {
	set +e
	echo "$1" | grep -E "^$answer$" > /dev/null
	[ $? -eq 0 ] && echo " *** Congrats. That is correct. *** " && echo && setlevel $(($level + 1)) || echo " %%% nope. nope. nope. %%% "
	set -e
}

function setlevel {
	[ -z "$1" ] || level="$1"
	[ ! -r $LEVELDIR/$level/config ] && echo "Needed level does not exist. Seems that you're an r2ninja now. Going to level 1." && level=1
	cd $LEVELDIR/$level/
	rm -r _workdir 2> /dev/null || true
	mkdir _workdir
	cp -- * _workdir/ 2> /dev/null || true
	cd _workdir
	rm config
	unset -v R2LRN_NAME R2LRN_TASK R2LRN_ANSWER R2LRN_HINT R2LRN_POSTEXEC R2LRN_POSTRESULT
	. ../config
	echo
	echo "Level $level $R2LRN_NAME"
	task="$R2LRN_TASK"
	answer="$R2LRN_ANSWER"
	[ -z "$answer" ] && answer="$(cat /dev/urandom | tr -dc A-Z0-9 | head -c80)"
	hint="$R2LRN_HINT"
	postexec="$R2LRN_POSTEXEC"
	postverify="$R2LRN_POSTRESULT"
	[ -z "$postverify" ] && postverify="$(cat /dev/urandom | tr -dc A-Z0-9 | head -c80)"

	echo "$task"
}


echo Welcome to r2lrn. Type exit, to exit.
echo Starting a limited shell. Only radare commands allowed.

setlevel 1

while true; do
	echo
	echo -n r2lrn $\ 
	read a
	command="$(echo $a | cut -d " " -f 1)"
	params="$(echo $a | grep " " | cut -d " " -f 2)"
	proclines=0
	
	case "$command" in
		exit) echo "OK, bye." && exit 0 ;;
		
		r2) proclines=1 ;;
		radare2) proclines=1 ;;
		ra*2) ;;
		
		ghidra) echo You win. Bye. && exit 69 ;;
		
		level) setlevel $params ; continue ;;
		hint) show_hint ; continue ;;
		answer) take_answer $params ; continue ;;
		help) echo "Available commands: answer, level, hint, exit, help" && continue ;;
		*) echo "Command not found. Did you mean to type ghidra $params?" && continue ;;
	esac
	
	[ -z "$(which $a)" ] && echo "Command not found." && continue
	$a | tee _console
	set +e
	cat _console | grep -E "^$answer$" > /dev/null
	[ $? -eq 0 ] && echo " *** Congrats. That is correct. *** " && echo && setlevel $(($level + 1)) && set -e && continue

	if [ ! -z "$postexec" ]; then
		$postexec | grep -E "^$postverify$" > /dev/null
		[ $? -eq 0 ] && echo " *** Congrats. You solved it correctly. *** " && echo && setlevel $(($level + 1)) && set -e && continue
	fi

	set -e

	
done
