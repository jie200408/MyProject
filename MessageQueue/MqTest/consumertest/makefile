test_consumer:consumer.cc ../../MqCommon/msg.pb.cc
	g++ -o $@ $^ -std=c++11 -lsqlite3 -lprotobuf -lgtest -g -lpthread
.PHONY:clean
clean:
	rm -f test_consumer