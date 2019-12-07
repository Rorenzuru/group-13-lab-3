#include <stdio.h>
#include <string.h>
#include <omnetpp.h>


using namespace omnetpp;

class Generator : public cSimpleModule
{
private:

  cPacket *packet;
  cMessage *event;

  int generated;

  protected:
    // The following redefined virtual function holds the algorithm.
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;

  public:
    Generator();
    virtual ~Generator();
  };

  Generator::Generator()
  {
    event = packet = nullptr;
  }


  Generator::~Generator()
  {
    cancelAndDelete(event);

  }


// The module class needs to be registered with OMNeT++
Define_Module(Generator);

void Generator::initialize()
{
    generated = 0;
    // Boot the process scheduling the initial message as a self-message.
    //packet = new cMessage("packet");
    simsignal_t interGenTime;
    event = new cMessage("event");
    scheduleAt(simTime()+par("interGenTime"), event);

}

void Generator::handleMessage(cMessage *msg)
{   // events are the only type of msg the generator will ever receive so:
    // generate new packet and send it to the queuing sub system and start
    // countdown for another event (distributed as Exponential)
      packet = new cPacket("packet");
      packet -> setByteLength((long)par("avgPcktSize"));
      send(packet, "out");
      generated++;
      //compute the new departure time:
      simsignal_t interGenTime;
      scheduleAt(simTime()+par("interGenTime"), event);

  }



  void Generator::refreshDisplay() const
  {
    char buf[40];
    sprintf(buf, "Packets sent: %d", generated);
    getDisplayString().setTagArg("t", 0, buf);

  }
