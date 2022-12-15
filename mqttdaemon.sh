#!/bin/bash
MQTTSUB="C:/Program Files/mosquitto/mosquitto_sub.exe"
while true
do
	"$MQTTSUB" -t "Paredes/ESP32" | while read -r payload
	do
		if [[ "$payload" == "1pmode" ]]; then
			echo "1P mode selected."
			./p1.sh
			echo "Concluded 1p mode"
			break
		elif [[ "$payload" == "2pmode" ]]; then
			echo "2P mode selected."
			./p2.sh
			echo "Concluded 2p mode"
		fi
	done
	
done