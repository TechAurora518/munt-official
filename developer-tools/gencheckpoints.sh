#! /bin/bash
set -e

#Fetch our private configuration variables and then hand the rest of the work off to python script
source developer-tools/private.conf
python developer-tools/python/gencheckpoints.py > src/data/chainparams_mainnet_static_checkpoint_data.cpp
