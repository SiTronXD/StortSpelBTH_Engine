#!/bin/bash

validation_file="vulkan_validations.txt"

if [ -z $1 ];
then 
    printf  "No argument, looking for text file named $validation_file in current directory\n\n"
else 
    validation_file="$1"
    printf  "Looking for given file '$validation_file'\n\n"
fi; 

if [ ! -f $validation_file ]
then
    printf  "File '$validation_file' does not exist! Exiting...\n\n"
    exit
fi; 

validation_file_content="$(cat $validation_file)"

unique_errors=$(awk -F ":| - " '!visited[$1]++ && !/^ / {c++; order[$1]=c; error[$1]=sprintf("%s \t %s", $3,$1)   } END {for (key in visited) { if( error[key] != "" ) printf "%*d:\t[%s] \t %s \n",  -3, order[key],visited[key], error[key] } }' "$validation_file" )
#unique_errors=$(awk -F ": handle|msgNum:| - " '!visited[$1]++ && !/^ / {c++; order[$1]=c; error[$1]=sprintf("%s \t %s \tHandle %s", $2,$1,$4)   } END {for (key in visited) { if( error[key] != "" ) printf "%*d:\t[%s] \t %s \n",  -3, order[key],visited[key], error[key] } }' "$validation_file" )

if false;
then 
    while IFS= read -r line; do
        
        msg_id="$(printf "$line" | awk -F "\t" "{print \$2}" | sed -E 's#\s##g')"
        msg_occourances=$(printf "$validation_file_content" | grep -e "$msg_id" | wc -l)
        msg_title="$(printf "$line" | awk -F "\t" "{print \$3}" | sed -E 's#\s##g')"
        
        printf "[%s] \t%s \t%s\n" "$msg_occourances" "$msg_id" "$msg_title"
    
    done <<< "$unique_errors"
fi

unique_errors="$(printf "$unique_errors" | sort -t ":" -nk1)"

#unique_errors2="$(printf "$unique_errors" | awk -F "MessageID = " '{ printf "%s \t %s \t %s\n", $1,$2,$3   }')" #$2,$1,$4

printf "Nr\tNrOf\t  ID\t\t Title:\n"
printf "$unique_errors" 

printf "\n\nI am Done!\n"
