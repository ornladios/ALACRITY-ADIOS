#!/bin/bash

echo "[TEST START] Filestore in legacy format using compressed index"
bin/test-filestore data/input/temp.dat tmp/tmp 0 data/output/temp
echo "[TEST END]"
echo

echo "[TEST START] Filestore in legacy format using inverted index"
bin/test-filestore data/input/temp.dat tmp/tmp 1 data/output/temp
echo "[TEST END]"
echo

echo "[TEST START] Serialization routines"
bin/test-serialize data/input/temp.dat tmp/tmp
echo "[TEST END]"
echo
