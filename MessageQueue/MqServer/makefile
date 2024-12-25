.PHONY: server
LFLAG= -lprotobuf -lsqlite3 -pthread -lmuduo_net -lmuduo_base -lz -g
server:server.cc ../MqCommon/msg.pb.cc ../MqCommon/proto.pb.cc ../MqThird/include/codec.cc ../MqCommon/user.pb.cc  ../MqCommon/admin.pb.cc
	g++ -g -std=c++11 $^ -o $@  $(LFLAG)
.PHONY:clean
clean:
	rm -f server