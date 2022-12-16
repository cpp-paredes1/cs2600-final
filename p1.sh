#!/bin/bash
MQTTSUB="C:/Program Files/mosquitto/mosquitto_sub.exe"
MQTTPUB="C:/Program Files/mosquitto/mosquitto_pub.exe"

legal=0
shouldExit=0
myinput="a"
function checkEnd(){
	clear
	"$MQTTPUB" -t "Paredes/P2Input" -m "getState"
	read -r payload
	printf "%s\n" "$payload"
	read -r payload
	printf "%s\n" "$payload"
	read -r payload
	printf "%s\n" "$payload"
	if [ "$payload" == "You win!" ] || [ "$payload" == "You lose!" ] || [ "$payload" == "Draw." ] || [ "$payload" == "quit" ]; then
		shouldExit=1
		exit
	fi
}
if [ $shouldExit == 0 ]; then
	while read -r payload
	do
		if [ "$payload" == "p2turn" ]; then
			legal=0
			checkEnd
			in1="$(($RANDOM % 3))"
			in2="$(($RANDOM % 3))"
			myinput="$in1,$in2"
			"$MQTTPUB" -t "Paredes/P2Input" -m "input:$myinput"
			read -r payload
			if [ "$payload" != "invalid" ] || [ "$payload" != "valid" ];then
				read -r payload
			fi
			if [ "$payload" == "invalid" ]; then
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
