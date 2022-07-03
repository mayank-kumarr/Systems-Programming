VERBOSE=$1
function log(){
    [ "$VERBOSE" == "-v" ]&&printf "\n$@\n\n"
}

REQ_HEADERS="Accept,Host,User-Agent"

curl -L -o example.html https://www.example.com/
log "Webpage fetched and saved as example.html"

curl -i http://ip.jsontest.com/
log "Response headers and IP printed"

rqh=(${REQ_HEADERS//,/ })
log "REQ_HEADERS: [${rqh[@]}]"

str=$(printf ".\"%s\", " "${rqh[@]}")
curl http://headers.jsontest.com/|jq "${str::-2}"
log "Required headers printed"

>valid.txt
>invalid.txt
for i in ./data1e/*.json;do
    chk=$(curl -d "json=$(cat ${i})" -X POST http://validate.jsontest.com/|jq '.validate')
    [ "$chk" == "true" ]&&echo $(basename $i)>>valid.txt||echo $(basename $i)>>invalid.txt
done

sort -o valid.txt valid.txt
log "valid.txt sorted"
sort -o invalid.txt invalid.txt
log "invalid.txt sorted"