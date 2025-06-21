#include <stdlib.h>
#include <iostream>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Valu.h"
#include "Valu___024unit.h"
#include <cstdlib>
#define MAX_SIM_TIME 200
#define VERIF_START_TIME 7
vluint64_t sim_time = 0;
vluint64_t posedge_cnt =0;
class AluScb;

void set_rnd_out_valid(Valu *dut, vluint64_t &sim_time){
    if (sim_time >= VERIF_START_TIME) {
        dut->in_valid = rand() % 2; // generate values 0 and 1
    }
}



void check_out_valid(Valu *dut, vluint64_t &sim_time){
	static unsigned char in_valid =0;
	static unsigned char in_valid_d =0;
	static unsigned char out_valid_exp =0;

	if(sim_time >= VERIF_START_TIME){
			out_valid_exp = in_valid_d;
			in_valid_d = in_valid;
			in_valid = dut->in_valid;
			if(out_valid_exp != dut->out_valid){
					  std::cout << "ERROR: out_valid mismatch, "
                << "exp: " << (int)(out_valid_exp)
                << " recv: " << (int)(dut->out_valid)
                << " simtime: " << sim_time << std::endl;

				}

		}
	}



void dut_reset(Valu *dut, vluint64_t &sim_time){
				
		dut->rst = 0;
		if(sim_time >= 3 && sim_time <6){
				dut->rst = 1;
      	dut->a_in = 0;
      	dut->b_in = 0;
      	dut->op_in = 0;
			}

	}

class AluInTx{
	public:
		uint32_t a;
		uint32_t b;
		enum Operation{
				add = Valu___024unit::operation_t::add,
				sub = Valu___024unit::operation_t::sub,
				nop = Valu___024unit::operation_t::nop
			}op;
	};
AluInTx* rndAluInTx(){
    //20% chance of generating a transaction
    if(rand()%5 == 0){
        AluInTx *tx = new AluInTx();
        tx->op = AluInTx::Operation(rand() % 3); // Our ENUM only has entries with values 0, 1, 2
        tx->a = rand() % 11 + 10; // generate a in range 10-20
        tx->b = rand() % 6;  // generate b in range 0-5
        return tx;
    } else {
        return NULL;
    }
}

class AluOutTx{
			public:
				uint32_t out;
	};
class AluScb{
	private:
		std::deque<AluInTx*> in_q;
	
	public:
		void writeIn(AluInTx *tx){
				in_q.push_back(tx);

			}
	void writeOut(AluOutTx* tx){
			if(in_q.empty()){
				std::cout <<"Fatal Error in AluScb: empty AluInTx queue" << std::endl;
				exit(1);

				}
			AluInTx* in;	
			in = in_q.front();
			in_q.pop_front();
			switch(in->op){

				case AluInTx::nop :
					std::cout << "Fatal error in AluScb, received NOP on input" << std::endl;	
					exit(1);
					break;
				case AluInTx::add :
					if(in->a +in->b != tx->out){
						std::cout << std::endl;
						std::cout << "  Expected: " << in->a + in->b
											<< "  Actual: " << tx->out << std::endl;
						std::cout << "  Simtime: " << sim_time << std::endl;
						}
						break;
				case AluInTx::sub:
					if(in->a -in->b!=tx->out){
							std::cout << std::endl;
							std::cout << "AluScb: sub mismatch" << std::endl;
							std::cout << "  Expected: " << in->a - in->b
												<< "  Actual: " << tx->out << std::endl;
							std::cout << "  Simtime: " << sim_time << std::endl;
						}
						break;
		}
		delete in;
		delete tx;
	}
};	
//AluInTx  * rndAluInTx(){
//	if(rand()%5 ==0){
//			AluInTx *tx = new AluInTx();
//			tx->op = AluInTx::Operation(rand()%3);
//			tx->a = rand()%11 +10;
//			tx->b = rand() % 6;
//			return tx;
//		} else{
//			return NULL;
//			}
//	}
class AluInDrv{
	private:
		Valu *dut;
	public:
		AluInDrv(Valu *dut){
			this->dut = dut;
			};
			void drive(AluInTx *tx){
					dut->in_valid = 0;
					if(tx != NULL){
							if(tx->op != AluInTx::nop){

									dut->in_valid = 1;
									dut->op_in = tx->op;
									dut->a_in = tx->a;
									dut->b_in = tx->b;

								}
							delete tx;
						}
				}
	};
class AluInMon{
	private:
		Valu *dut;
		AluScb *scb;
	public:
		AluInMon(Valu *dut, AluScb *scb){
				this->dut = dut;
				this->scb = scb;
			}	
		void monitor(){
				if(dut->in_valid == 1){
						AluInTx *tx = new AluInTx();
						tx->op = AluInTx::Operation(dut->op_in);
						tx->a = dut->a_in;
						tx->b = dut->b_in;
						scb->writeIn(tx);
					}

			}

	};
class AluOutMon{
	private:
		Valu *dut;
		AluScb *scb;
	public:
		AluOutMon(Valu *dut, AluScb *scb){
				this->dut = dut;
				this->scb = scb;
			}
			void monitor(){
					if(dut->out_valid ==1 ){
						AluOutTx *tx = new AluOutTx();
						tx->out = dut->out;
						scb->writeOut(tx);
						}

				}

	};


int main(int argc, char** argv, char** env) {
		srand(time(NULL));
		Verilated::commandArgs(argc,argv);
    Valu *dut = new Valu;

    Verilated::traceEverOn(true);
    VerilatedVcdC *m_trace = new VerilatedVcdC;
    dut->trace(m_trace, 5);
    m_trace->open("waveform.vcd");

		AluInTx *tx;
		
		AluInDrv *drv = new AluInDrv(dut);	
		AluScb	 *scb = new AluScb();
		AluInMon *inMon = new AluInMon(dut,scb);
		AluOutMon *outMon = new AluOutMon(dut,scb);

    while (sim_time < MAX_SIM_TIME) {
				dut_reset(dut,sim_time);	
				dut->clk ^=1;
				dut->eval();
				if(dut->clk == 1){
					if(sim_time >= VERIF_START_TIME){
							tx =rndAluInTx();
							drv->drive(tx);
							inMon->monitor();
							outMon->monitor();
						}
						}
					
       	m_trace->dump(sim_time);
       	sim_time++;
    }

    m_trace->close();
    delete dut;
		delete outMon;
		delete inMon;
		delete scb;
		delete drv;
    exit(EXIT_SUCCESS);

}

