#!/usr/bin/bash

set -euo pipefail

THIS_FOLDER=$(dirname "$0")
for FILENAME in ${THIS_FOLDER}/*.sbcs; do
   ${THIS_FOLDER}/../as2.py "${FILENAME}" "${FILENAME%.*}.bin"
done