>$1
for i in {1..150};do 
    num=$RANDOM
    for j in {1..9};do
        num="${num},$RANDOM"
    done
    echo $num>>$1
done
[[ `cut -d, -f$2 $1|grep $3|wc --lines`>0 ]]&&echo "YES"||echo "NO"