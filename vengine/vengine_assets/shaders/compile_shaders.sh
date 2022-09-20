#!/bin/bash

#OBSOLETE; Do not use this... use the cmake file isntead...

debug="TRUE"
debug="FALSE"

if [ ! -f "history_blob" ]; 
then 
    touch history_blob
else 
    myHistory="$(cat history_blob)";
fi; 

for shader in shader.*; do

    spv_name=$(printf "$shader"| sed -E 's#.*\.(.*)#\1.spv#g')
    shd_name=$(printf "$shader"| sed -E 's#.*\/(.*)#\1#g')
    begining_of_id="$spv_name:$shd_name:"

    fetchedId="$(printf "$myHistory" | grep ${shd_name} | sed -En "s#$begining_of_id(.*)#\1#p")"

    if [ "$debug" == "TRUE" ]; then
        printf "$spv_name\n"
        printf "$shd_name\n"
        printf "$(printf "$myHistory" | grep "${shd_name}" )"
    fi;    
    if [ "$debug" == "TRUE" ]; then
        printf "\n\n$(md5sum -b  "${shd_name}")\n"

        printf "fetched:$fetchedId\n"
        printf "calced :$(md5sum -b  ${shd_name})\n"
    fi;

    if [ -z "$(printf "$myHistory" | grep "${shd_name}")" ] ; 
    then
        glslangValidator -V ${shader}        
        printf "%s%s\n" "$begining_of_id" "$(md5sum -b  ${shd_name})" >> history_blob
        #printf "$shd_name was changed, recompile!\n"
        if [ "$debug" == "TRUE" ]; then
            printf "A\n"
        fi;

    elif [ "$(printf "$myHistory" | grep "${shd_name}")" ] && [ ! -f "$spv_name" ];
    then 
        glslangValidator -V ${shader}
        rowContentsToRemove="$(printf "$myHistory" | grep "${shd_name}" | sed -E 's#\.#\\.#g' | sed -E 's#\*#\\*#g') "
        #printf "content: $rowContentsToRemove\n\n"
        for lines in $rowContentsToRemove; do
            #printf "content: $lines\n"
            sed -Ei "/^${lines}/d" history_blob   
            #sed -Ei '/^vert\.spv:shader\.vert:187af7771d3ae72a50d9678004e97c87 \*shader\.vert/d' history_blob
        done

        printf "%s%s\n" "$begining_of_id" "$(md5sum -b  ${shd_name})" >> history_blob
        if [ "$debug" == "TRUE" ]; then
            printf "B\n"
        fi;

    
    #Check if file has changed
    elif [[ ! "$fetchedId" == $(md5sum -b  "${shd_name}") ]]; 
    then        

        # Generate spirv, remove old line and insert new into historyblob
        glslangValidator -V "$shader"
        sed -i "s#${begining_of_id}.*#${begining_of_id}$(md5sum -b  ${shd_name})#" history_blob        
    
    fi;

done

#glslangValidator -V shader.vert
#glslangValidator -V shader.frag