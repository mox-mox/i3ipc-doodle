#!/bin/sh
# Install to /usr/lib/systemd/system-sleep/example.sh
# mox
case $1/$2 in
  pre/*)
    echo "FOOOOOOOOOOOOOO Going to $2..."
	#sleep 10
    ;;
  post/*)
    echo "FOOOOOOOOOOOOOO Waking up from $2..."
    ;;
esac
