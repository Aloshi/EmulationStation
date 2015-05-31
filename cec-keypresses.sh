#!/usr/bin/env bash
#
# react to cec keypresses in the jankiest way possible
#
# Author: Dave Eddy <dave@daveeddy.com>
# Date: 10/15/2013
# Licens: MIT
# Tested on: Raspberry pi with libcec compiled from soure

onright() {
 	echo 'right button pressed'
}
onleft() {
	echo 'left button pressed'
}
ondown() {
	echo 'down button pressed'
}
onup() {
	echo 'up button pressed'
}
onselect() {
	echo 'select button pressed'
}
onplay() {
	echo 'play button pressed'
}
onpause() {
	echo 'pause button pressed'
}
onforward() {
	echo 'forward button pressed'
}
onbackward() {
	echo 'back button pressed'
}

filter() {
	perl -nle 'BEGIN{$|=1} /key pressed: (.*) \(.*\)/ && print $1'
}

echo as | cec-client | filter | \
while read cmd; do
	case "$cmd" in
		right) onright;;
		left) onleft;;
		down) ondown;;
		up) onup;;
		select) onselect;;
		play) onplay;;
		pause) onpause;;
		forward) onforward;;
		backward) onbackward;;
		*) echo "unrecognized button ($cmd)";;
	esac
done
