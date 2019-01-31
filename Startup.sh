#!/usr/bin/env bash

useIPV6=true

if [ $# -ne 1 ]; then
	printf "Usage: ${BASH_SOURCE} <interface>\n"
	exit -1
elif ! sudo true ; then
	printf "Usage: sudo ${BASH_SOURCE} <interface>\n"
fi

sudo sysctl -w net.ipv4.conf.all.send_redirects=0
sudo sysctl -w net.ipv4.ip_forward=1

if [ "${useIPV6}" = true ]; then
	sudo sysctl -w net.ipv6.conf.all.forwarding=1
fi

sudo iptables  -t nat -A PREROUTING -i $1 -p tcp --dport 80 -j REDIRECT --to-port 8080
sudo iptables  -t nat -A PREROUTING -i $1 -p tcp --dport 443 -j REDIRECT --to-port 8080

if [ "${useIPV6}" = true ]; then
	sudo ip6tables -t nat -A PREROUTING -i $1 -p tcp --dport 80 -j REDIRECT --to-port 8080
	sudo ip6tables -t nat -A PREROUTING -i $1 -p tcp --dport 443 -j REDIRECT --to-port 8080
fi

mitmproxy --mode transparent --showhost -s ./MentiMITMEvent.py --ssl-insecure
