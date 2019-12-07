#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <fstream>

using namespace std;
using namespace omnetpp;

class TXQueue : public cSimpleModule
{
private:


  cQueue queue; // models the queue sub-system
  int max_len; //maximum length reached by the queue


  /**
  // a reference to the output transmission channel, we avoid using a
  // self message to notify the queue has processed the packet.
  // To compute what we've previously called 'finishedService' we use the
  // data rate of the channel and we call event 'endOfTx'.
  // In this case the next packet is sent when the current packet has
  // completed its transmission.
  */

  cChannel *txChannel;
  cMessage *endOfTx;
  simtime_t txFinishTime; //absolute time
  simtime_t txDelay;

  // stats collection (additional, just as a self-check)
  // limited size queue: need to know the number of arrived and lost packets
  // to compute the loss probability
  double pcktArrived;
  double pcktDropped;
  double pLoss;


protected:
  // The following redefined virtual function holds the algorithm.
  virtual void initialize() override;
  virtual void handleMessage(cMessage *msg) override;
  virtual void refreshDisplay() const override;
  virtual void finish() override;
  void measureQueue();

public:
  TXQueue();
  virtual ~TXQueue();
};

TXQueue::TXQueue()
{
  endOfTx = nullptr;
  txChannel = nullptr;
}


TXQueue::~TXQueue()
{
  cancelAndDelete(endOfTx);
}
// The module class needs to be registered with OMNeT++
Define_Module(TXQueue);

void TXQueue::initialize()
{/*
  The queue is just waiting for the arrival of the first packet,
  I may have to set the state of the queue (length of waiting line)
  and if the server is operating or not
  */
  // Initialize variables.
  endOfTx = new cMessage("endOfTx");
  max_len = par("maxLength");
  txChannel = gate("out") -> getTransmissionChannel();
  txFinishTime = simTime();

}

void TXQueue::handleMessage(cMessage *msg){
  /*
  //
  */
  if (msg == endOfTx) {
    // if another packet is available in the queue
    if (!queue.isEmpty()) {
      // remove the packet from the tx queue
      msg = (cMessage *)queue.pop();
      send(msg, "out");
      // compute the end of the transmission of the current packet
      txFinishTime = txChannel -> getTransmissionFinishTime();
      scheduleAt(txFinishTime, endOfTx);
      // evaluate the transmission delay
      // txDelay = txFinishTime - simTime();
    }
  }

  /*
  // the message is: 'a packet arrived to the queue',
  // we insert it in the queue,
  // then we verify the status of the server:
  // if is channel is idle -> send packet
  // otherwise -> store packet in the queue
  */
  else{

    pcktArrived++;

    // check the queue length, if has reached the maximum allowed value,
    // the pckt cannot be and thus will be dropped, in both cases
    // we increase the counter of arrived pckts
    // not scheduled and end of transmission message
    if (!endOfTx -> isScheduled()){
      send(msg, "out");
      // compute the end of the transmission of the current packet
      txFinishTime = txChannel -> getTransmissionFinishTime();
      scheduleAt(txFinishTime, endOfTx);
    }
    else if (queue.getLength() <  max_len) {
      queue.insert(msg);

    } // increment the number of dropped packets
      // and delete the packet
    else{
      pcktDropped++;
      delete(msg);
    }
  }
}



void TXQueue::refreshDisplay() const{
  char buf[40];
  sprintf(buf, "queue size: %d", queue.getLength());
  getDisplayString().setTagArg("t", 0, buf);

}



void TXQueue::finish() {
  // finish
  /*
  measureQueue(); // we average min and max the qlength accumulator for the simulation time
  tx_queue_log_file << "#"<<endl;
  tx_queue_log_file << "#"<<endl;
  tx_queue_log_file << "#"<<endl;
  tx_queue_log_file << "# Final Measurements"<<endl;
  tx_queue_log_file << "# [average queue length]:\t"<<queue_len_avg<<endl;
  tx_queue_log_file << "# [Packet Loss Probability]:\t"<<pLoss<<endl;
  */
  pLoss = pcktDropped/pcktArrived;

  EV << "\n***********************************************************\n";
  EV << getFullName() <<") Packet Loss Probability:\t" << pLoss <<"\n";
  EV << "\n***********************************************************\n";
}
