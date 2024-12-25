all:client

client:client.cc ../MqCommon/proto.pb.cc ../MqThird/include/codec.cc ../MqCommon/msg.pb.cc ../MqCommon/user.pb.cc ../MqCommon/admin.pb.cc
	g++ -o $@ $^ -std=c++11 -lmuduo_net -lmuduo_base -lpthread -lz -lprotobuf -g -lstdc++fs


.PONHY:clean
clean:
	rm -f client