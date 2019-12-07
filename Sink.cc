#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <fstream>
#include <math.h>


using namespace omnetpp;
using namespace std;

class Sink : public cSimpleModule
{
private:
  int arrived; //number of packets reaching H2

  //average delay, inter arrival time (IAT) and throughput
  double avgDelay;
  double avgIAT;
  double avgThroughput;

  simtime_t delay;
  simtime_t intArrTime;
  simtime_t prevArrived;



  // Accumulator variables for the computation of the averages of
  // 1) delay
  // 2) throughput
  // 3) inter arrival time of packets

  // And also:

  // 4) variation coefficient (we use an additive accumulator variable)

  double accDelay;
  double accThroughput;
  double accIAT;
  double accIATsqrd;
  double varCoeff;

  int pcktLengthBytes;
  cPacket* pckt;



  // log file creation
  ofstream delay_log_file;
  ofstream throughput_log_file;
  ofstream IAT_log_file;
  ofstream var_coeff_log_file;

protected:
  // The following redefined virtual function holds the algorithm.
  virtual void initialize() override;
  virtual void handleMessage(cMessage *msg) override;
  virtual void refreshDisplay() const override;
  virtual void finish() override;

  //measurements methods
  void measureDelay(simtime_t delay);
  void measureIAT_VC(simtime_t intArrTime);
  void measureThroughput(int pcktLengthBytes);
  void measureVarCoeff();
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
  The packets have arrived to the sink who collects data
*/

  arrived = 0;
  accDelay=0;
  avgDelay=0;
  prevArrived = simTime();
  intArrTime = simTime();
  accIAT = 0;
  accIATsqrd = 0;
  varCoeff = 0;
  avgThroughput=0;




/*
  //Average packet delay log file
  ostringstream delay_log_file_name;
  delay_log_file_name << "log-delay.dat";
  delay_log_file.open(delay_log_file_name.str());
  delay_log_file << "# LOG from module "<<getFullPath()<<endl;
  delay_log_file << "# [average packet delay]"<<endl;

  // Throughput log file
  ostringstream throughput_log_file_name;
  throughput_log_file_name << "log-throughput.dat";
  throughput_log_file.open(throughput_log_file_name.str());
  throughput_log_file << "# LOG from module "<< getFullPath()<<endl;
  throughput_log_file << "# [average throughput]"<<endl;

  // average inter arrival time log file
  ostringstream IAT_log_file_name;
  IAT_log_file_name << "log--time.dat";
  IAT_log_file.open(IAT_log_file_name.str());
  IAT_log_file << "# LOG from module "<< getFullPath()<<endl;
  IAT_log_file << "# [average inter arrival time]"<<endl;


  // average variation coeff log file
  ostringstream var_coeff_log_file_name;
  var_coeff_log_file_name << "log-var-coeff.dat";
  var_coeff_log_file.open(var_coeff_log_file_name.str());
  var_coeff_log_file << "# LOG from module "<< getFullPath()<<endl;
  var_coeff_log_file << "# [Variation coefficient]"<<endl;

  */
}



void Sink::handleMessage(cMessage *msg)
{ // Packests exits the Switch and goes in the sync who represents the
  // host 2
  arrived++;
  // Secure casting attempt via method check_and_cast<>()
  // measure the pckt length in Bytes
  pckt = check_and_cast<cPacket *>(msg);
  pcktLengthBytes = pckt ->getByteLength();

  // delay is given by difference between the creation time and the current time
  delay = simTime() - msg->getCreationTime();
  // Inter arrival time is given by difference between the time of arrival
  // of the previous packet and the current time
  intArrTime = simTime() - prevArrived;
  prevArrived = simTime();
  measureDelay(delay);
  measureIAT_VC(intArrTime);
  measureThroughput(pcktLengthBytes);

/*
  delay_log_file << simTime() <<"\t"<< avgDelay << endl;
  throughput_log_file << simTime() << "\t" << avgThroughput << endl;
  IAT_log_file << simTime() << "\t" << avgIAT << endl;
  var_coeff_log_file << simTime() << "\t" << varCoeff <<endl;
*/
  delete(msg);
}


void Sink::measureDelay(simtime_t delay){
  // method for computation of average delay

  accDelay += delay.dbl();
  avgDelay = accDelay/arrived;
}


void Sink::measureIAT_VC(simtime_t intArrTime){
  // method for computation of average the inter arrival time
  accIAT += intArrTime.dbl();
  avgIAT = accIAT/arrived;

  accIATsqrd += pow(intArrTime.dbl(),2);
  // The following formula is equivalent to
  // var_coeff = std_deviation / average

/*
                                 (ACCUMULATOR_IAT_SQRD/numReceived)
COEFFICIENTEVARIAZIONE =   sqrt(----------------------------------- -1)
                                    (pow(interarrival_mean,2)

*/


  double den = pow(accIAT,2);
  double fraction = (accIATsqrd*arrived)/den;
  varCoeff = sqrt( fraction -1);
}


void Sink::measureThroughput(int pcktLengthBytes){
  // method for computation of average delay
  // avg throughput formula (in Mbps)
  accThroughput += (pcktLengthBytes * 8 * 1e-6);
  avgThroughput = accThroughput / simTime();
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
  EV << "SINK: final average inter arrival time:\t" << avgIAT << " s";
  EV << "\n***********************************************************\n";
  EV << "SINK: final variation coefficient:\t" << varCoeff;
  EV << "\n***********************************************************\n";
  EV << "SINK: final average throughput:\t" << avgThroughput << " Mbps";
  EV << "\n***********************************************************\n";
/*
  // close the file at the end
  delay_log_file.close();
  throughput_log_file.close();
  IAT_log_file.close();
  var_coeff_log_file.close();
  */
}
