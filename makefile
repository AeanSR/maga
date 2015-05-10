maga.exe	: mdp.o demo.o
	g++ -o maga.exe mdp.o demo.o
mdp.o	: mdp.cpp
	g++ -c mdp.cpp -O3
demo.o	: demo.cpp
	g++ -c demo.cpp -O3