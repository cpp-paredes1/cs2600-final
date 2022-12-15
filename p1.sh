#!/bin/bash
MQTTSUB="C:/Program Files/mosquitto/mosquitto_sub.exe"
MQTTPUB="C:/Program Files/mosquitto/mosquitto_pub.exe"

legal=0
myinput="a"
# while true
# do
	"$MQTTSUB" -t "Paredes/ESP32" | while read -r payload
	do
		if [[ "$payload" == "p1turn" ]]; then
			while [[ $legal == 0 ]]
			do
				in1="$(($RANDOM % 3 + 1))"
				in2="$(($RANDOM % 3 + 1))"
				myinput="$in1$in2"
				echo $myinput
				"$MQTTPUB" -t "Paredes/P2Input" -m "$myinput"
				"$MQTTSUB" -t "Paredes/ESP32" | while read -r payload
				do
					if [[ $payload == "invalid" ]]; then
						echo "Invalid input."
						payload="p1turn"
						break
					elif [[ $payload == "valid" ]]; then
						legal=1
						break
						echo "broke"
					fi
					break
				done
				break
			done
			break
		elif [[ "$payload" == "winner" ]]; then
			echo "You won!"
			exit
		elif [[ "$payload" == "loser" ]]; then
			echo "You lost!"
			exit
		fi
	done
# done
