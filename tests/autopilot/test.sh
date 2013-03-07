#!/bin/sh

for name in `autopilot list tests | head -n -3 | tail -n +3`
do
  autopilot run $name || exit 1
done
