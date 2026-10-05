#!/bin/bash

# Check if an argument was provided
if [ -z "$1" ]; then
    echo "Usage: $0 <search_term>"
    exit 1
fi

# Perform the grep command using the first argument
grep -r --color=always "$1" --exclude-dir={build,external} .
