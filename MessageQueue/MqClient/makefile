all:consume_client publish_client
consume_client:consume_client.cc ../MqCommon/proto.pb.cc ../MqThird/include/codec.cc ../MqCommon/msg.pb.cc
	g++ -o $@ $^ -std=c++11 -lmuduo_net -lmuduo_base -lpthread -lz -lprotobuf -g
publish_client:publish_client.cc ../MqCommon/proto.pb.cc ../MqThird/include/codec.cc ../MqCommon/msg.pb.cc
	g++ -o $@ $^ -std=c++11 -lmuduo_net -lmuduo_base -lpthread -lz -lprotobuf -g

.PONHY:clean
clean:
	rm -f publish_client consume_client