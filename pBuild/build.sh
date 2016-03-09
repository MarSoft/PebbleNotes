#!/bin/bash
set -e
echo "Building Pebble project..."
cd ~/build/${PEBBLE_PROJECT_PATH}
# pebble build isn't used because it fails
yes | ~/pebble-dev/${PEBBLE_SDK}/bin/pebble build
