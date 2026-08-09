#!/usr/bin/env fish

# 1. Parse Arguments
# v/verbose: boolean flag to show all output
argparse 'v/verbose' -- $argv
or return

set -l verbose (set -q _flag_v; and echo 1; or echo 0)

set PASS 0
set FAIL 0
set TOTAL 0

set_color --bold blue
echo "--- Nytrogen Compiler Test Suite ---"
set_color normal

# Iterate through multiple directories
# This expansion finds all .ny files in tests/basic/ and tests/complex/
for test_file in tests/{basic,complex}/**/*.ny
    set TOTAL (math $TOTAL + 1)

    set_color yellow
    echo -n "[$TOTAL] Testing "(basename $test_file)"... "
    set_color normal

    # Capture output to a temp file
    set -l tmp_output (mktemp)

    # Run the driver and capture EVERYTHING (stdout and stderr)
    ./run.sh $test_file > $tmp_output 2>&1
    set -l exit_code $status

    if test $exit_code -eq 0
        set_color green
        echo "PASS"
        set PASS (math $PASS + 1)

        # If verbose flag is on, show output even on pass
        if test $verbose -eq 1
            set_color white
            cat $tmp_output
            echo ""
        end
    else
        set_color --bold red
        echo "FAIL"
        set FAIL (math $FAIL + 1)

        # Always show output on failure if we want to see "logical errors"
        # or errors during compilation.
        set_color white
        echo "------------------------------------------------------"
        echo "LOG OUTPUT FOR $test_file:"
        cat $tmp_output
        echo "------------------------------------------------------"
        set_color normal
    end

    rm $tmp_output
end

# Final Report
echo "------------------------------------"
set_color --bold
if test $FAIL -eq 0
    set_color green
    echo "ALL TESTS PASSED! ($PASS/$TOTAL)"
else
    set_color red
    echo "TEST SUITE FAILED ($FAIL/$TOTAL)"
end
set_color normal

if test $FAIL -gt 0
    exit 1
end
