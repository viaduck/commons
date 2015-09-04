#!/bin/sh
BASE=`pwd`
cd include/ThinXchange
cog.py -d -I ${BASE}/tools/ThinXchange/ -c @${BASE}/tools/ThinXchange/messages.list
cd -