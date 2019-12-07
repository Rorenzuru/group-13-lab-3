#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <fstream>


using namespace omnetpp;
using namespace std;

class Sink : public cSimpleModule
{
private:
  int arrived; //number of samples
  double minDelay;
  double maxDelay;
  double avgDelay;
  simtime_t delay;
  double accumulatorDelay;


  double avgThroughput;

  int pcktLengthBytes;
  cPacket* pckt;

  simtime_t serviceTime; //exponentially distributed (1/mu), mu < lambda
  cMessage *finshedService;

  // log file creation
  ofstream delay_log_file;

protected:
  // The following redefined virtual function holds the algorithm.
  virtual void initialize() override;
  virtual void handleMessage(cMessage *msg) override;
  virtual void refreshDisplay() const override;
  virtual void finish() override;
  void measureDelay(simtime_t delay);

public:
  Sink();
  virtual ~Sink();

};


Sink::Sink()
{
  pckt = nullptr;

}


Sink::~Sink()
{

}



// The module class needs to be registered with OMNeT++
Define_Module(Sink);

void Sink::initialize()
{/*
  The customers have arrived to the sink who collects data
  ex. : number of customer arrived
  */
  minDelay = 1e5;
  maxDelay = -1;

  arrived = 0;

  accumulatorDelay=0;
  avgDelay=0;
  avgThroughput=0;

  WATCH(minDelay);
  WATCH(maxDelay);
  WATCH(avgDelay);


  // Initialize variables.
  ostringstream delay_log_file_name;
  // create file name in the form: log-NAME-MODULE.dat
  delay_log_file_name << "log-delay.dat";
  // open the file
  delay_log_file.open(delay_log_file_name.str());
  // add header with a description
  delay_log_file << "# LOG from module "<<getFullPath()<<endl;
  //Average queue length measurement
  delay_log_file << "# [average packet delay]"<<endl;
}

void Sink::handleMessage(cMessage *msg)
{ //Customer exits the queue and goes in the sync
  arrived++;
  // Secure casting attempt via method check_and_cast<>()
  pckt = check_and_cast<cPacket *>(msg);

  pcktLengthBytes = pckt ->getByteLength();
  EV <<"Length of pckt in Bytes:\t"<< pcktLengthBytes <<endl;

  // delay is given by difference between the creation time  and the current time
  delay = simTime() - msg->getCreationTime();
  measureDelay(delay);
  delay_log_file << simTime() <<"\t"<<avgDelay<<endl;

  // avg throughput formula (in bps)
  avgThroughput = (arrived * pcktLengthBytes * 8 * 1e-6)/simTime();

  delete(msg);



}


void Sink::measureDelay(simtime_t delay){
  //method for computation of minimum, maximum and average delay
  if (delay.dbl() < minDelay)
  minDelay = delay.dbl();

  if (delay.dbl() > maxDelay)
  maxDelay = delay.dbl();

  accumulatorDelay += delay.dbl();
  avgDelay = accumulatorDelay/arrived;
}



void Sink::refreshDisplay() const
{
  char buf[40];
  sprintf(buf, "Packets arrived: %d", arrived);
  getDisplayString().setTagArg("t", 0, buf);

}

void Sink::finish() {
  // finish

  EV << "\n***********************************************************\n";
  EV << "SINK: final average delay:\t" << avgDelay << " s";
  EV << "\n***********************************************************\n";
  EV << "SINK: final average throughput:\t" << avgThroughput << " Mbps";
  EV << "\n***********************************************************\n";

  delay_log_file.close(); // close the file at the end
}
