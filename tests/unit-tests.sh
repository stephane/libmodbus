#!/bin/bash

client=$( find -maxdepth 1 -executable -type f -iname '*unit-test-client*')
if [ "$client" == "" ]; then
    echo "Unit-test-client is not found"
    return -1;
fi
echo "Client executable found"

server=$( find -maxdepth 1 -executable -type f -iname '*unit-test-server*')
if [ "$client" == "" ]; then
    echo "Unit-test-server is not found"
    return -1;
fi
echo "Server executable found"


client_log=unit-test-client.log
server_log=unit-test-server.log

if [ -f $client_log ]; then rm $client_log; fi;
if [ -f $server_log ]; then rm $server_log; fi;

echo "Starting server"
$server 2>&1 > $server_log &
sleep 1
echo "Server started"

echo "Starting client"
$client | tee $client_log


