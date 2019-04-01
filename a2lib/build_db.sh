#!/bin/bash

#Move old database to backup
DATE=`date +%Y%m%d`
mv main.db main.${DATE}.db
#Build database
sqlite3 main.db < settings.sql
