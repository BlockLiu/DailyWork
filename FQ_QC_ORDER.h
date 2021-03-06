/*
 * Weighted Round Robin
 *
 * Variables:
 * queue_num_: number of Class of Service (CoS) queues
 * thresh_: ECN marking threshold
 * mean_pktsize_: configured mean packet size in bytes
 * marking_scheme_: Disable ECN (0), Per-queue ECN (1) and Per-port ECN (2)
 */

#ifndef ns_FQ_QC_ORDER_h
#define ns_FQ_QC_ORDER_h

#define MAX_QUEUE_NUM 8

#define DISABLE_ECN 0
#define PER_QUEUE_ECN 1
#define PER_PORT_ECN 2

#include <string.h>
#include "queue.h"
#include "config.h"
#include <unordered_map>
#include <set>
#include <math.h>
#include <stdlib.h>
#include "sketchForKnown.h"

using namespace std;

class FQ_QC_ORDER : public Queue {
	protected:
		void enque(Packet*);	// enqueue function
		Packet* deque();        // dequeue function

        PacketQueue **q_;		// underlying multi-FIFO (CoS) queues
		int mean_pktsize_;		// configured mean packet size in bytes
		int thresh_;			// single ECN marking threshold
		int queue_num_;			// number of CoS queues. No more than MAX_QUEUE_NUM
		int marking_scheme_;	// Disable ECN (0), Per-queue ECN (1) and Per-port ECN (2)
		unordered_map<int, double> message_id;
		unordered_map<int, int> flow_count;
		set<int> count_flow_in[MAX_QUEUE_NUM];
		int pkt_num[MAX_QUEUE_NUM];
		int pkt_sum[MAX_QUEUE_NUM];

		unordered_map<int, int> current_flow[MAX_QUEUE_NUM];
		int rr_times[MAX_QUEUE_NUM];

		CMSketch_known *sketch;

	public:
		FQ_QC_ORDER()
		{
			queue_num_=MAX_QUEUE_NUM;
			thresh_=65;
			mean_pktsize_=1500;
			marking_scheme_=PER_PORT_ECN;

			//Bind variables
			bind("queue_num_", &queue_num_);
			bind("thresh_",&thresh_);
			bind("mean_pktsize_", &mean_pktsize_);
            bind("marking_scheme_", &marking_scheme_);

			//Init queues
			q_=new PacketQueue*[MAX_QUEUE_NUM];
			for(int i=0 ; i < MAX_QUEUE_NUM ; i++)
			{
				q_[i] = new PacketQueue;
				pkt_num[i] = 0;
				pkt_sum[i] = 0;
				rr_times[i] = 0;
			}

			sketch = new CMSketch_known();
		}

		~FQ_QC_ORDER()
		{
			for(int i=0;i<MAX_QUEUE_NUM;i++)
			{
				delete q_[i];
			}
			delete[] q_;
		}

	protected:
		//Return total queue length (bytes) of all the queues
		int TotalByteLength()
		{
			int bytelength = 0;
			for(int i = 0 ; i < MAX_QUEUE_NUM ; i++)
				bytelength += q_[i]->byteLength();
			return bytelength;
		}
		
};

#endif
