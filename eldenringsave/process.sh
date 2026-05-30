while true
do 
	if tasklist.exe|grep -iq "ChatGPT.exe"
	then cp backup1.txt testfile.txt
	     echo "Done sucesfully"
	     break
	fi
	sleep 5
done
