#!/bin/sh

client_log=unit-test-client.log
server_log=unit-test-server.log

rm -f $client_log $server_log

echo "Starting server"
./unit-test-server > $server_log 2>&1 &

sleep 1

echo "Starting client"
./unit-test-client > $client_log 2>&1
rc=$?

killall unit-test-server
exit $rc

