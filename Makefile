all:server.cpp client.cpp log account
	g++ server.cpp -o server
	g++ client.cpp -o client
log:
	mkdir log
account:
	mkdir account
clean:
	rm server client Account_List log/* account/*
