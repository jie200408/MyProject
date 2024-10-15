test:test.cc ../../MqCommon/msg.pb.cc ../../MqCommon/proto.pb.cc
	g++ -o $@ $^ -std=c++11 -lsqlite3 -lprotobuf -lgtest -g
.PHONY:clean
clean:
	rm -f test