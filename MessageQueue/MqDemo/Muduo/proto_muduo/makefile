all:client server
client:protobuf_client.cc request.pb.cc ../../include/proto/codec.cc
	g++ -o $@ $^ -std=c++11 -lmuduo_net -lmuduo_base -lpthread -lz -lprotobuf
server:protobuf_server.cc request.pb.cc ../../include/proto/codec.cc
	g++ -o $@ $^ -std=c++11 -lmuduo_net -lmuduo_base -lpthread -lz -lprotobuf