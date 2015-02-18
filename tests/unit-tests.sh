#!/bin/sh

client_log=unit-test-client.log
server_log=unit-test-server.log

client=$(find -maxdepth 1 -executable -type f -iname '*unit-test-client*')
if [ -z "$client" ]; then
    echo "Unit-test-client is not found"
    exit 1
fi
echo "Client executable found"

server=$(find -maxdepth 1 -executable -type f -iname '*unit-test-server*')
if [ -z "$server" ]; then
    echo "Unit-test-server is not found"
    exit 1
fi
echo "Server executable found"

rm -f $client_log $server_log

echo "Starting server"
$server > $server_log 2>&1 &
server_pid=$!
sleep 1
echo "Server started"

echo "Starting client"
$client | tee $client_log

kill $server_pid
