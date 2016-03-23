#Jordan Giacone
#CS 3800

#!/bin/bash
clear
choice=1
echo Welcome
echo ------------------------------------------------
while [ $choice -ne 4 ]
do
echo Please select a menu option:
echo 1 - Print the Ancestry tree of the current process
echo 2 - See who is online
echo 3 - See all processes a user is running
echo 4 - Exit
read choice  

case $choice in
1) echo " "
   procid=$$
   ps -acefl > processList
   echo The ancestry tree for this process is...
   echo " "
   parentprocid=0
   echo " "
   echo $procid
   
   while (( $procid != 1 ))
   do
     echo "  |"
     parentprocid=$(awk -v id=$procid '{if ($4==id) print;}' processList)
     procid=$(echo $parentprocid | cut -f5 -d ' ')	
     echo $procid
   done
   rm processList;;
2) echo " "
   echo "Users currently online are: "
   who | cut -f1 -d ' ' | sort | uniq -u;;
3) echo " "
   who | cut -f1 -d ' ' | sort | uniq -u > userlist
   
   index=0
   cat userlist | while read line
   do
     index=$(($index + 1))
     echo $index\) $line 
   done
   
   totalUsers=$(wc -l < userlist)
   echo Please choose a user from the above list
   read userChoice 
   
   while (( $userChoice <= 0 )) || (( $userChoice > $totalUsers ))
   do
     echo $userChoice is an invalid option. Please try again.
     read userChoice   
   done
   
   echo " "
   user=$(sed "$userChoice q;d" userlist)
   echo You chose $user
   echo " "
   
   ps -acefl > processList
   awk -v usr=$user '{if ($3==usr) print;}' processList 
   
   totalUsers=0
   rm userlist
   rm processList;;
4) echo " "
   echo Thank you for using this program!;;
*) echo $choice is not a valid choice. Please enter a number 1 through 4
esac
echo " "
done
