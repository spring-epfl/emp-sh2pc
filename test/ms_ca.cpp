#include "emp-sh2pc/emp-sh2pc.h"
#include "../emp-tool/emp-tool/io/io_channel.h"
#include "../emp-tool/emp-tool/emp-tool.h"
using namespace emp;
using namespace std;
#include <chrono>
#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

void test_mpsi(int party, int input[][128], int query, int keywords, int documents) {

	Integer A[query];
	for(int i = 0; i < query; ++i)
		A[i] = Integer(32, input[0][i], ALICE);

	Integer B[documents][keywords];
	for(int i = 0; i < documents; ++i){
		for(int j = 0; j < keywords; ++j){
			B[i][j] = Integer(32, input[i][j], BOB);
		}
	}

	Bit x_ms;
	Bit f_psm[documents];
	Bit tmp[query];

	for(int i = 0; i < documents; ++i){
		f_psm[i] = 1;
		for(int j = 0; j < query; ++j){
			for(int k = 0; k < keywords; ++k){
				tmp[j] = (tmp[j] | (A[j] == B[i][k]));
			}
			f_psm[i] = (f_psm[i] & tmp[j]); //= 1 if all Aj match a Bik
		}
		x_ms = x_ms | f_psm[i]; //is there one doc such that f psm is 1
	}
	cout << "Party[" << party << "] X-MS F-PSM?\t" << x_ms.reveal<bool>(ALICE)<<endl;
}

void test_campsi(int party, int input[][128], int query, int keywords, int documents) {

	Integer A[query];
	for(int i = 0; i < query; ++i)
		A[i] = Integer(32, input[0][i], ALICE);

	Integer B[documents][keywords];
	for(int i = 0; i < documents; ++i){
		for(int j = 0; j < keywords; ++j){
			B[i][j] = Integer(32, input[i][j], BOB);
		}
	}

	Integer ca_ms;
	Integer f_psm[documents];
	Integer tmp[query];

	ca_ms = Integer(32, 0, XOR);
	for(int i = 0; i < documents; ++i){
		f_psm[i] = Integer(32, 1, XOR);
		for(int j = 0; j < query; ++j){
			tmp[j] = Integer(32, 0, XOR);
			for(int k = 0; k < keywords; ++k){
				vector<Bit> eq_vect(1, (A[j] == B[i][k]));
				Integer eq = Integer(eq_vect);
				eq.resize(32, false);
				tmp[j] = tmp[j] | eq;
			}
			f_psm[i] = f_psm[i] & tmp[j]; //= 1 if all Aj match a Bik
		}
		ca_ms = ca_ms + f_psm[i]; //is there one doc such that f psm is 1
	}
	cout << "Party[" << party << "] CA-MS F-PSM?\t" << ca_ms.reveal<int>(ALICE)<<endl;
}

int main(int argc, char** argv) {
	int port, party;
	parse_party_and_port(argv, &party, &port);
	NetIO * io = new NetIO(party==ALICE ? nullptr : argv[4], port, false);

	int documents = 2;
	if(argc > 3)
		documents = atoi(argv[3]);
	int keywords = 128;
	int query = 8;

	int inputs[documents][128];

	srand(time(NULL));
	int seed1 = rand();
	int seed2 = rand();

	if(party == 1) {
		srand(seed1);
		for(int i=0; i < query; ++i){
			inputs[0][i] = rand(); // << 13 | rand()%(1<<13);
			//LHS is 31bit as RAND_MAX=2^31-1
			//shifted by 13 bits
			//RHS is in [0, 2^13]
			//overall 44bits
		}
	}

	if(party == 2) {
		srand(seed2);
		for(int i=0; i < documents; ++i){
			for(int j = 0; j < keywords; ++j){
				inputs[i][j] = rand();// << 13 | rand()%(1<<13);
			}
		}
	}

	auto start = chrono::high_resolution_clock::now();

	setup_semi_honest(io, party);
	//test_mpsi(party, inputs, query, keywords, documents);
	test_campsi(party, inputs, query, keywords, documents);
	finalize_semi_honest();

	auto end = chrono::high_resolution_clock::now();
	double time_taken =
		chrono::duration_cast<chrono::nanoseconds>(end - start).count();
	time_taken *= 1e-9;
  	cout << "Time taken for party[" << party << "]: "
		<< fixed
		<< time_taken << setprecision(9) << " sec" << endl;

	//cout << "Num and (?)" << CircuitExecution::circ_exec->num_and()<<endl;
	io->print_counter();
	delete io;
	return 0;
}
