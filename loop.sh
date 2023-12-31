#!/bin/sh

seed=$(conway)

while true; do
    echo "$seed" | conway -print
    echo ""
    sleep .5s
    seed=$(echo "$seed" | conway)
done

