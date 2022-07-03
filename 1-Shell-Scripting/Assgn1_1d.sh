mkdir -p ./files_mod
for i;do
    sed = $i|sed 'N;s/\n/ /'|sed 's/ /,/g'>./files_mod/$(basename $i)
done