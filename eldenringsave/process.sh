while true
do 
	if tasklist.exe|grep -iq "ChatGPT.exe"
	#chatgpt is example here use elden ring instead.
	#use your save and backfiles instead of txt files.
	#will update to full working version later.

	then cp backup1.txt testfile.txt
	     echo "Done sucesfully"
	     break
	fi
	sleep 5
done
