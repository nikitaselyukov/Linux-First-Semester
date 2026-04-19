#!/bin/bash

PIPE_NAME="message_pipe"

if [ ! -p "$PIPE_NAME" ]; then
    echo "$PIPE_NAME does not exist"
    exit 1
fi

while true; do
    if read -r message <"$PIPE_NAME"; then
        echo "$(date) - $message"
    fi
done
