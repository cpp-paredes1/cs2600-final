#!/bin/bash
MQTTSUB="C:/Program Files/mosquitto/mosquitto_sub.exe"
MQTTPUB="C:/Program Files/mosquitto/mosquitto_pub.exe"

legal=0
shouldExit=0
myinput="a"
function checkEnd(){
	"$MQTTPUB" -t "Paredes/P2Input" -m "getState"
	read -r payload
	printf "$payload\n"
	if [ "$payload" == "You win!" ] || [ "$payload" == "You lose!" ] || [ "$payload" == "quit" ]; then
		shouldExit=1
		exit
	fi
}
if [ $shouldExit == 0 ]; then
	while read -r payload
	do
		if [ "$payload" == "p2turn" ]; then
			clear
			legal=0
			checkEnd
			read -p "Input a row and column format: Column,Row? " myinput </dev/tty
			"$MQTTPUB" -t "Paredes/P2Input" -m "input:$myinput"
			read -r payload
			if [ "$payload" == "invalid" ]; then
				echo "Invalid input."
				"$MQTTPUB" -t "Paredes/P2Input" -m "resendp2turn"
			elif [ "$payload" == "valid" ]; then
				checkEnd
				legal=1
				payload="p1turn"
			fi
		elif [ "$payload" == "quit" ]; then
			shouldExit=1
		fi
	done
fi
