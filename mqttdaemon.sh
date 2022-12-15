#!/bin/bash
MQTTSUB="C:/Program Files/mosquitto/mosquitto_sub.exe"
runProj="C:/Users/minno/Documents/Code/CS2600/FINAL PROJ/cool/bin/finalproj.exe"
cmd="C:/WINDOWS/system32/cmd.exe"
while true
do
	"$MQTTSUB" -t "Paredes/ESP32" | while read -r payload
	do
		echo $payload
		if [[ "$payload" == "1pmode" ]]; then
			echo "1p"
			./p1.sh
			echo "we oout"
			break
		elif [[ "$payload" == "2pmode" ]]; then
			echo "2p"
			./p2.sh
			break
		fi
	done
done