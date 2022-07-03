num=$1
for (( i=2; i<$1; i++ ));do
    while [ $((num%$i)) == 0 ];do
        printf "$i "
        num=$((num/$i))
    done
done
[[ $num == $1 ]]&&printf "1 $1"