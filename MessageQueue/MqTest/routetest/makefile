test_route:route.cc ../../MqCommon/msg.pb.cc
	g++ -o $@ $^ -std=c++11 -lsqlite3 -lprotobuf -lgtest -g
.PHONY:clean
clean:
	rm -f test_route