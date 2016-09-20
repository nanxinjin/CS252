#!/bin/bash

#DO NOT REMOVE THE FOLLOWING TWO LINES
git add $0 >> .local.git.out
git commit -a -m "Lab 2 commit" >> .local.git.out
git push

#Your code here
SCORE=0
PASSWORD=`cat $1`
LENGTH=${#PASSWORD}
SUGGESTION="Password Suggestion:"
nl='
'
if [ $LENGTH -lt 6 -o $LENGTH -gt 32 ]; then
	echo "Error: Password length invalid."
else
	let SCORE=$SCORE+$LENGTH
	#echo $SCORE
	#echo $1
	#echo $PASSWORD
	if egrep -qs "[#$+%@]" passwordfile ;then
		let SCORE=$SCORE+5
		#echo "inside #$+%@"
	#	echo $SCORE
	fi
	if egrep -qs "[0-9]" passwordfile ;then
		let SCORE=$SCORE+5
		#echo "inside [0-9]"
	#	echo $SCORE
	fi
	if egrep -qs "[A-Za-z]" passwordfile ;then
		let SCORE=$SCORE+5
		#echo "inside [A-Za-z]"
	#	echo $SCORE
	fi

	if egrep -qs "[a-z][a-z][a-z]" passwordfile ;then
		let SCORE=$SCORE-3
		SUGGESTION="$SUGGESTION${nl}Don't use consecutive lowercase characters."
	#	echo "inside [a-z][a-z][a-z]"
	fi
	if egrep -qs "[A-Z][A-Z][A-Z]" passwordfile ;then
		let SCORE=$SCORE-3
		SUGGESTION="$SUGGESTION${nl}Don't use consecutive uppercase characters."
	#	echo "inside [A-Z][A-Z][A-Z]"
	fi
	if egrep -qs "[0-9][0-9][0-9]" passwordfile ;then
		let SCORE=$SCORE-3
		SUGGESTION="$SUGGESTION${nl}Don't use consecutive numbers."
	#	echo "[0-9][0-9][0-9]"
	fi
	if egrep -qs "([A-Za-z])\1+" passwordfile ;then
		let SCORE=$SCORE-10
		SUGGESTION="$SUGGESTION${nl}Don't use same alphanumeric character one after another."
	#	echo "([A-Za-z])+"
	fi
	echo "Password Score:" $SCORE
	echo $SUGGESTION
	#EXTRA CREDIT

	echo "Hello $USER, " > tmp-message
	echo>>tmp-message
	echo "Your password is $PASSWORD" >> tmp-message
	echo>>tmp-message
	echo "Your score is $SCORE" >> tmp-message
	echo>>tmp-message
	echo $SUGGESTION >> tmp-message
	/usr/bin/mailx -s "This is your password and your score" $USER < tmp-message
	echo "Message sent."
fi
