#!/bin/bash

set -eux

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

$test_bin >$TEMPDIR/backtrace.log 2>$TEMPDIR/backtrace.err &
pid=$!
sleep .3
kill -sSIGUSR1 $pid
sleep .3
kill -sSIGINT $pid
wait $pid || true

sed -i "s,$SRCDIR/,/home/username/qbm/core/tests/," $TEMPDIR/backtrace.log
sed -i "s,$SRCDIR/,/home/username/qbm/core/tests/," $TEMPDIR/backtrace.err

diff $SRCDIR/backtrace.log $TEMPDIR/backtrace.log
diff $SRCDIR/backtrace.err $TEMPDIR/backtrace.err
rm -rf ${TEMPDIR?}
