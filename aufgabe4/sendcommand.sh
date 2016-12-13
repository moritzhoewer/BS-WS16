#!/bin/bash
while [ 1 ]
do 
	read -p "Enter command : " cmd
	echo $cmd > $1
done
