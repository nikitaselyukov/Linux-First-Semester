#!/bin/bash

PIPE_NAME="message_pipe"

echo "Hello, world!" >"$PIPE_NAME"
sleep 1
echo "This is a test message." >"$PIPE_NAME"
sleep 1
echo "Another message!" >"$PIPE_NAME"
