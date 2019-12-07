
simple Generator
{
    parameters:
        volatile double interGenTime @unit(s);
        int avgPcktSize; //[Bytes];
        @display("i=block/source");
    gates:
        output out;
}


simple TXQueue
{
    parameters:
        // volatile double serviceTime @unit(s);
        int maxLength; //[packets]
        @display("i=block/queue");
    gates:
        input in;
        output out;
}



simple Sink
{
    parameters:
    @display("i=block/queue");
    gates:
        input in;
}


module Host
{
  gates:
    output out;

  submodules:
    generator: Generator;
    queue : TXQueue;

  connections:
      generator.out --> queue.in;
      queue.out --> out;
}


network Ethernet
{
    parameters:
    double dataRate @unit(Mbps);

    submodules:
       generatorHost: Generator;
       queueHost : TXQueue;
       queueSwitch : TXQueue;
       sink: Sink;

    connections:
        generatorHost.out --> queueHost.in;
        queueHost.out --> {datarate = 100Mbps; delay = 1us;} --> queueSwitch.in;
        queueSwitch.out --> {datarate = dataRate; delay = 1us;} --> sink.in;
}
