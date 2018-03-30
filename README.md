# wt-bootstrap-example
Example code of using Bootstrap theme and Witty (libwt) written in C++

This example is written as "spagetti code".
Every functional block for main elements is grouped in "{ }"

Written for libwt 3.3.4

Bootstrap theme: sb-admin-2 (https://startbootstrap.com/template-overviews/sb-admin-2/)

#Compile

g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/main.d" -MT"src/main.o" -o "src/main.o" "../src/main.cpp"

#Link

g++  -o "wt-bootstrap"  ./src/main.o   -lboost_system -lwt -lwthttp


#Run

./wt-bootstrap --docroot . --http-address 0.0.0.0 --http-port 8080

#Test

Start browser and type address http://127.0.0.1:8080/
