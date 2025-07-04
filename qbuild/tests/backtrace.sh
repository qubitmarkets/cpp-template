#!/bin/bash

set -eu

test_bin=${1?}
test_bin=$(readlink -f $test_bin)

cd $(dirname ${BASH_SOURCE})
SRCDIR=$PWD

TEMPDIR=$(mktemp -d /tmp/backtrace.XXXXXX)

if [[ ! -f $test_bin ]]; then
    echo "Could not find $test_bin"
fi

if [[ $test_bin =~ "release.gcc" ]]; then
    echo "Skipping test, not supported combination"
    exit 0
fi

set -x
$test_bin >$TEMPDIR/backtrace.log 2>$TEMPDIR/backtrace.err &
pid=$!
# Wait for startup
sleep .1

kill -sSIGUSR1 $pid
# Wait for SIGUSR1 to be handled.  Can be longer if debug symbols take a long time to load.
if [[ $test_bin =~ "sanitize" ]]; then
    sleep .8
else
    sleep .5
fi

kill -sSIGINT $pid
# Wait for SIGINT to be handled
wait $pid || true
set +x

sed -i "s,$SRCDIR/,/home/username/qbm/core/tests/," $TEMPDIR/backtrace.log
sed -i "s,$SRCDIR/,/home/username/qbm/core/tests/," $TEMPDIR/backtrace.err

diff $SRCDIR/backtrace.log $TEMPDIR/backtrace.log
if ! diff -q $SRCDIR/backtrace.err $TEMPDIR/backtrace.err; then
    echo "Error: backtrace.err does not match"
    echo "---------------------------------------"
    echo "Expected:"
    cat $SRCDIR/backtrace.err
    echo "---------------------------------------"
    echo "Got:"
    cat $TEMPDIR/backtrace.err
    exit 1
fi
rm -rf ${TEMPDIR?}
