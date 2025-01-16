#!/bin/bash

#Generate random number 1-100
target_number=$((RANDOM % 100 + 1))

#Initial number
guess=0

#Guess number
while [ "$guess" -ne "$target_number" ]; do
	read -p "Print the number you guess: " guess

	#Judge
	if [ "$guess" -lt "$target_number" ]; then
		echo "It's too small. Please try again."
	elif [ "$guess" -gt "$target_number" ]; then
	        echo "It's too big. Please try again."
        else 
		echo "Congratulations! You are right!"
 	fi
done	
