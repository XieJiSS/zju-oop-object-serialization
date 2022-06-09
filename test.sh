#!/bin/bash

cd "$(dirname "$0")"

echo "[test_daemon] Cleaning previously generated files..."
./clean_generated.sh

echo "[test_daemon] Generating Makefile..."
cmake .

echo "[test_daemon] Building..."
make -j

cd ./tests

SERIALIZE_TEST_PASSED_COUNT=3

echo "[test_daemon] Running tests..."
time ./test_binary || SERIALIZE_TEST_PASSED_COUNT=$(($SERIALIZE_TEST_PASSED_COUNT-1))
read -p "Press [Enter] to continue testing... (enter)"
time ./test_xml || SERIALIZE_TEST_PASSED_COUNT=$(($SERIALIZE_TEST_PASSED_COUNT-1))
read -p "Press [Enter] to continue testing... (enter)"
time ./test_xml_b64 || SERIALIZE_TEST_PASSED_COUNT=$(($SERIALIZE_TEST_PASSED_COUNT-1))

if [ "$SERIALIZE_TEST_PASSED_COUNT" -eq "3" ]; then
    echo "[test_daemon] All tests cases (test_binary, test_xml, test_xml_b64) passed"
    exit 0
else
    echo "[test_daemon] Only $SERIALIZE_TEST_PASSED_COUNT tests passed, check logs for more information"
    exit 1
fi
