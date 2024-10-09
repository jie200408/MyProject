all: dict_client dict_server

dict_client:dict_client.cc
	g++ -o $@ $^ -std=c++11 -lmuduo_net -lmuduo_base -lpthread

dict_server:dict_server.cc
	g++ -o $@ $^ -std=c++11 -lmuduo_net -lmuduo_base -lpthread