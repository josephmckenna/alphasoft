#!/bin/bash

cd "${AGRELEASE}/a2lib"
#Move old database to backup
DATE=`date +%Y%m%d`
if [ -f main.db ]; then
   mv main.db main.${DATE}.db
fi
#Build database
sqlite3 main.db < settings.sql
