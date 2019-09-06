#!/bin/bash
# (C) Kirils Solovjovs, 2019
# GNU GPLv3, preserve authorship

set -e

function cleanup {
	[ -z "$(jobs -p)" ] || kill $(jobs -p)
}

function stay {
	echo
	[ -z "$(jobs -p)" ] || trap stay SIGINT
}

trap cleanup EXIT
trap stay SIGINT

OLDDIR="$(pwd)"
cd "$(dirname "$0")"
APPDIR="$(pwd)"
LEVELDIR="$APPDIR/levels"

cd "$OLDDIR"

export HISTFILE="$APPDIR/.history"
export HISTSIZE=10
history -r

function show_hint {
	[ -z "$hint" ] && echo "Sorry. Level developer was a meany and didn't include a hint." && return 0
	echo "Hint: $hint"

}


function take_answer {
	set +e
	echo "$*" | grep -E "^$answer$" > /dev/null
	[ $? -eq 0 ] && echo " *** Congrats. That is correct. *** " && echo && setlevel $nextlevel || echo " %%% nope. nope. nope. %%% "
	set -e
}

function setlevel {
	[ -z "$1" ] || level="$1"
	[ ! -r "$LEVELDIR/$level/config" ] && echo "Needed level does not exist. Seems that you're an r2ninja now. Going to level 1." && level=1
	cd $LEVELDIR/$level/
	rm -r .workdir 2> /dev/null || true
	mkdir .workdir
	cp -- * .workdir/ 2> /dev/null || true
	cd .workdir
	rm config
	unset -v R2LRN_NAME R2LRN_TASK R2LRN_ANSWER R2LRN_HINT R2LRN_POSTEXEC R2LRN_POSTRESULT R2LRN_COMMAND R2LRN_NEXTLEVEL
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
	commandanswer="$R2LRN_COMMAND"
	[ -z "$commandanswer" ] && commandanswer="$(cat /dev/urandom | tr -dc A-Z0-9 | head -c80)"
	nextlevel="$R2LRN_NEXTLEVEL"
	[ -z "$nextlevel" ] && nextlevel=$(($level + 1))

	echo "$task"
}


echo "Welcome to r2lrn. Type exit, to exit."
echo
echo "Starting limited BASH-LIKE shell. Type \"help\" to see possible r2lrn commands."
echo "Only radare tools (r2, rabin2, etc...) and internal r2lrn commands can be used."
echo
echo "If you decide to use interactive r2 for any of the challenges,"
echo "exit it as soon as you get the answer (or submit answer manually)."

level=1

setlevel $1

while true; do
	rm -- .console* 2> /dev/null || true
	prompt="$USER@$HOSTNAME ./levels/$level $ "
	read -e -p "$prompt" a
	history -s "$a"
	history -w
	command="$(echo $a | cut -d " " -f 1)"
	params="$(echo $a | grep " " | cut -d " " -f 2-)"
	proclines=0
	
	case "$command" in
		exit) echo "OK, bye." && exit 0 ;;
		
		"") continue ;;
		
		r2) proclines=1 ;;
		radare2) proclines=1 ;;
		ra*2) ;;
		
		ghidra) echo You win the game. Bye. && exit 69 ;;
		
		level) setlevel $params ; continue ;;
		cd) echo "Use 'level' to change level" && continue ;;
		hint) show_hint ; continue ;;
		answer) take_answer $params ; continue ;;
		ls) ls -l ; continue ;;
		help) echo "Available commands: answer, level, hint, ls, exit, help" && continue ;;
		debug) set | grep ^R2LRN_ ; continue ;;
		*) echo "Command not found. Did you mean to type ghidra $params?" && continue ;;
	esac
	
	[ -z "$(which $a)" ] && echo "Command not found." && continue
	
	set +e
		$a | tee .console
	set -e
	
	if [ $proclines -eq 1 ] ; then
		set +e
		cat .console | tr  -d '\200-\377' \
			| sed -r "s/\x1B\[([0-9]{1,2}(;[0-9]{1,3})*)?[mGK]//g;s/ +/ /gm" \
			| tr -d '\000-\011' | tr '\r' '\n' |sed '/^$/d' > .console.clear
		cat .console.clear | grep -vE "^\[0x[0-9a-f]+\]> " | grep -v "^\^D$" > .console.answers
		cat .console.clear | sed -E '/^\[0x[0-9a-f]+\]>.*/! s/.*//;s/\[0x[0-9a-f]+\]> //' \
			| uniq | awk '!/^$/ {line=$0} /^$/ {print line} END {print line}' \
			| uniq | grep -v "^$" | grep -v "^ex" | grep -v "^q" > .console.cmds
		set -e
	else
		cp .console .console.clear
		cp .console .console.answers
		touch .console.cmds
	fi
	
	set +e

		if [ ! -z "$postexec" ]; then
			$postexec | grep -E "^\s*$postverify\s*$" > /dev/null
			[ $? -eq 0 ] && echo " *** Congrats. You solved it correctly. *** " && echo && setlevel $nextlevel
		else
			tail -1 .console.answers | grep -E "^\s*$answer\s*$" > /dev/null
			if [ $? -eq 0 ]; then
				echo " *** Congrats. That is correct. *** " && echo && setlevel $nextlevel
			else
				tail -1 .console.cmds | grep -E "^\s*$commandanswer\s*$" > /dev/null
				[ $? -eq 0 ] && echo " *** Congrats. You solved it correctly. *** " && echo && setlevel $nextlevel
			fi
		fi
	
	set -e
done
