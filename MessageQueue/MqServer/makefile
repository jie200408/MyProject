.PHONY: server
LFLAG= -lgtest -lprotobuf -lsqlite3 -pthread -lmuduo_net -lmuduo_base -lz
server:server.cc ../MqCommon/msg.pb.cc ../MqCommon/proto.pb.cc ../MqThird/include/codec.cc
	g++ -g -std=c++11 $^ -o $@  $(LFLAG)
.PHONY:clean
clean:
	rm -f server