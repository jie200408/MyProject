all:consume_client publish_client

LFLAG= -L../MqThird/lib -lgtest -lprotobuf -lsqlite3 -pthread -lmuduo_net -lmuduo_base -lz -g

consume_client:consume_client.cc ../MqCommon/proto.pb.cc ../MqThird/include/codec.cc ../MqCommon/msg.pb.cc
	g++ -g -std=c++11 $^ -o $@  $(LFLAG)
publish_client:publish_client.cc ../MqCommon/proto.pb.cc ../MqThird/include/codec.cc ../MqCommon/msg.pb.cc
	g++ -g -std=c++11 $^ -o $@  $(LFLAG)

.PONHY:clean
clean:
	rm -f publish_client consume_client