#!/bin/sh

client_log=unit-test-client.log
server_log=unit-test-server.log

rm -f "$client_log" "$server_log"

echo "Starting server"
./unit-test-server > $server_log 2>&1 &
US_PID=$!

sleep 1

echo "Starting client"
./unit-test-client > $client_log 2>&1
rc=$?

if [ $rc != 0 ]; then
    echo "Tests failed, for more details: cat $client_log $server_log" >&2
fi

echo "Stopping test server (ok to fail if it already exited)" >&2
if (command -v killall) >/dev/null ; then
    killall unit-test-server
fi
if [ 0"$US_PID" -gt 0 ] 2>/dev/null ; then
    kill $US_PID
fi

exit $rc

