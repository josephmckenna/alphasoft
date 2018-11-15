
#pip install wordcloud

#Make a file with full git log messages
git log --pretty="format:%B" &> log.txt
#remove common names from messages
cat log.txt | sed 's/KO//g' | sed 's/AC//g' | sed 's/LM//g' > no_names.txt
#Remove merges from messages
grep -v Merge no_names.txt &> no_merge.txt
#Make a world cloud:
wordcloud_cli --width 1920 --height 1080 --text no_merge.txt --imagefile wordcloud.png
