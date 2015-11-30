#! /bin/bash
experiment-cli get -r | python $(dirname "$0")/run_characterization.py $@
