///
/// Definition of ZB_MonitoringAndControl class.
///

/*
 * $Author$
 * $Id$
 * $Date$
 * $Revision$
 * */

#include "ZB_MonitoringAndControl.h"

using namespace std;

// Default constructor
ZB_MonitoringAndControl::ZB_MonitoringAndControl(string device): Thread()
{
    //ctor
    if(device.compare("") != 0){
        txrx_ = new ZB_Frame_TXRX(device);
        frameId_ = 1;
        run_ = false;
        auto_update_nodes_ = true;
    }
        //return this;
    /*else
        return 0;*/
}

// Constructor
ZB_MonitoringAndControl::ZB_MonitoringAndControl(ZB_Frame_TXRX::API_MODE api_mode, string device): Thread()
{
    //ctor
    if(device.compare("") != 0){
        txrx_ = new ZB_Frame_TXRX(api_mode, device);
        frameId_ = 1;
        run_ = false;
        auto_update_nodes_ = true;
    }
        //return this;
    /*else
        return 0;*/
}

ZB_MonitoringAndControl::~ZB_MonitoringAndControl()
{
    //dtor
    stopMonitoring();
    delete txrx_;
}

ZB_MonitoringAndControl::ZB_MonitoringAndControl(const ZB_MonitoringAndControl& other)
{
    //copy ctor
}

ZB_MonitoringAndControl& ZB_MonitoringAndControl::operator=(const ZB_MonitoringAndControl& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

// retrieveCommandsResponseBuffer method
bool ZB_MonitoringAndControl::retrieveCommandsResponseBuffer(unsigned char frameID, Resumed_AT_Response& commandResponse)
{
    bool found = false;

    lock();
    {
        map<unsigned char, Resumed_AT_Response>::iterator it = commandsResponse_buffer_.find(frameID);
        if(it != commandsResponse_buffer_.end()){

            commandResponse = it->second;
            commandsResponse_buffer_.erase(it);
            found = true;
        }
    }
    unlock();

    return found;
}

// accessNodeIOSample method
bool ZB_MonitoringAndControl::accessNodeIOSample(string node, API_IO_Sample* io_sample)
{
    bool found = false;

    lock();
    {
        map< std::string, API_IO_Sample*>::iterator it = nodeSample_map_.find(node);

        if(it != nodeSample_map_.end()){

            *io_sample = *it->second;
            found = true;
        }

        if(!found){

            for(it = nodeSample_map_.begin(); it != nodeSample_map_.end(); it++){

                cout << "Node: " << it->first << endl;
                if (it->second->getSourceNetworkAddress().compare(node) == 0 ||
                    it->second->getSourceAddress().compare(node) == 0){

                    io_sample = it->second;
                    found = true;
                    break;
                }
            }
        }
    }
    unlock();

    return found;
}

// Discover network nodes method
unsigned char ZB_MonitoringAndControl::discoverNetworkNodes(string NI)
{
    //API_AT_Command *atCommand = new API_AT_Command(frameId_, "ND", NIorMY);
    API_AT_Command *atCommand = new API_AT_Command(frameId_, int2Hex((unsigned int)API_AT_Command::EXEC_CRE_NODE_DISCOVERY), NI);
    /*Resumed_AT_Response resumedResponse;
    resumedResponse.commandStatus = API_AT_CommandResponse::UNKOWN_STATUS;
    resumedResponse.parameterValueType = NO_TYPE;
    resumedResponse.parameterValue = 0;*/

    txrx_->sendMessage(atCommand->getFrame());

    delete atCommand;
    return incrementFrameID() - 0x1;
}

// setSleepMode method
unsigned char ZB_MonitoringAndControl::setSleepMode(string networkAddr, API_AT_Command::SleepMode sleepMode)
{
    API_AT_RemoteCommand *at_remoteCommand;
    string parameter = "";

    parameter.append(1, sleepMode);

    if(networkAddr.compare("") == 0){
        cout << "No node network address provided!" << endl;
        return 0x0;
    }
    else{

        at_remoteCommand = new API_AT_RemoteCommand(frameId_, "SM", parameter);
        setRemoteAddressing(at_remoteCommand, networkAddr);
        at_remoteCommand->setRemoteCommandOption(API_AT_RemoteCommand::APPLY_CH);
        txrx_->sendMessage(at_remoteCommand->getFrame());

        delete at_remoteCommand;
        return incrementFrameID() - 0x1;
    }
}

// setNumberOfSleepPeriods method
unsigned char ZB_MonitoringAndControl::setNumberOfSleepPeriods(string networkAddr, unsigned int numberOfPeriods)
{
    /*Resumed_AT_Response resumedResponse;
    resumedResponse.commandStatus = API_AT_CommandResponse::UNKOWN_STATUS;
    resumedResponse.parameterValueType = NO_TYPE;
    resumedResponse.parameterValue = 0;*/

    if(networkAddr.compare("") == 0){

        API_AT_Command *atCommand;
        atCommand = new API_AT_Command(frameId_, "SN", int2Hex(numberOfPeriods));
        txrx_->sendMessage(atCommand->getFrame());

        /*lock();
        commandsResponse_buffer_[frameId_] = resumedResponse;
        unlock();*/

        delete atCommand;
        return incrementFrameID() - 0x1;
    }
    else{

        API_AT_RemoteCommand *at_remoteCommand;
        at_remoteCommand = new API_AT_RemoteCommand(frameId_, "SN", int2Hex(numberOfPeriods));
        setRemoteAddressing(at_remoteCommand, networkAddr);
        at_remoteCommand->setRemoteCommandOption(API_AT_RemoteCommand::APPLY_CH);

        txrx_->sendMessage(at_remoteCommand->getFrame());

        /*lock();
        commandsResponse_buffer_[frameId_] = resumedResponse;
        unlock();*/

        delete at_remoteCommand;
        return incrementFrameID() - 0x1;
    }
}

// setSleepPeriod method
unsigned char ZB_MonitoringAndControl::setSleepPeriod(string networkAddr, unsigned int sleepPeriod)
{
    if(networkAddr.compare("") == 0){

        API_AT_Command *atCommand;
        atCommand = new API_AT_Command(frameId_, "SP", int2Hex(sleepPeriod));
        txrx_->sendMessage(atCommand->getFrame());

        delete atCommand;
        return incrementFrameID() - 0x1;
    }
    else{

        API_AT_RemoteCommand *at_remoteCommand;
        at_remoteCommand = new API_AT_RemoteCommand(frameId_, "SP", int2Hex(sleepPeriod));
        setRemoteAddressing(at_remoteCommand, networkAddr);
        at_remoteCommand->setRemoteCommandOption(API_AT_RemoteCommand::APPLY_CH);

        txrx_->sendMessage(at_remoteCommand->getFrame());

        delete at_remoteCommand;
        return incrementFrameID() - 0x1;
    }
}

unsigned char ZB_MonitoringAndControl::setTimeBeforeSleep(string networkAddr, unsigned int timeBeforeSleep)
{
    if(networkAddr.compare("") == 0){

        API_AT_Command *atCommand;
        atCommand = new API_AT_Command(frameId_, "ST", int2Hex(timeBeforeSleep));
        txrx_->sendMessage(atCommand->getFrame());

        delete atCommand;
        return incrementFrameID() - 0x1;
    }
    else{

        API_AT_RemoteCommand *at_remoteCommand;
        at_remoteCommand = new API_AT_RemoteCommand(frameId_, "ST", int2Hex(timeBeforeSleep));
        setRemoteAddressing(at_remoteCommand, networkAddr);
        at_remoteCommand->setRemoteCommandOption(API_AT_RemoteCommand::APPLY_CH);

        txrx_->sendMessage(at_remoteCommand->getFrame());

        delete at_remoteCommand;
        return incrementFrameID() - 0x1;
    }
}

// setSleepOptions method
unsigned char ZB_MonitoringAndControl::setSleepOptions(string networkAddr, API_AT_Command::SleepOptions sleepOptions)
{
    API_AT_RemoteCommand *at_remoteCommand;
    string parameter = "";
    //Resumed_AT_Response resumedResponse;
    parameter.append(1, sleepOptions);
    /*resumedResponse.commandStatus = API_AT_CommandResponse::UNKOWN_STATUS;
    resumedResponse.parameterValueType = NO_TYPE;
    resumedResponse.parameterValue = 0;*/

    if(networkAddr.compare("") == 0){
        cout << "No node network address provided!" << endl;
        return 0x0;
    }
    else{

        at_remoteCommand = new API_AT_RemoteCommand(frameId_, "SO", parameter);
        setRemoteAddressing(at_remoteCommand, networkAddr);
        at_remoteCommand->setRemoteCommandOption(API_AT_RemoteCommand::APPLY_CH);
        txrx_->sendMessage(at_remoteCommand->getFrame());

        /*lock();
        commandsResponse_buffer_[frameId_] = resumedResponse; // Populating the map with a new frame ID regarding a AT command.
        unlock();*/

        delete at_remoteCommand;
        return incrementFrameID() - 0x1;
    }
}

// setNodeIdentifier method
unsigned char ZB_MonitoringAndControl::setNodeIdentifier(string networkAddr, string nodeIdent)
{
    if(networkAddr.compare("") == 0){

        API_AT_Command *atCommand = new API_AT_Command(frameId_, "NI", nodeIdent);
        txrx_->sendMessage(atCommand->getFrame());

        delete atCommand;
        return incrementFrameID() - 0x1;
    }
    else{

        API_AT_RemoteCommand *at_remoteCommand;
        at_remoteCommand = new API_AT_RemoteCommand(frameId_, "NI", nodeIdent);
        setRemoteAddressing(at_remoteCommand, networkAddr);
        at_remoteCommand->setRemoteCommandOption(API_AT_RemoteCommand::APPLY_CH);

        txrx_->sendMessage(at_remoteCommand->getFrame());

        delete at_remoteCommand;
        return incrementFrameID() - 0x1;
    }
}

// endpointCyclicSleepConfiguration method
unsigned char ZB_MonitoringAndControl::endpointCyclicSleepConfiguration(string networkAddr, unsigned long sleepPeriod)
{
    return '\0';
}

unsigned char ZB_MonitoringAndControl::setIOSampleRate(string networkAddr, unsigned int ioSampleRate)
{
    if(networkAddr.compare("") == 0){

        return 0x0;
    }
    else{

        API_AT_RemoteCommand *at_remoteCommand;
        at_remoteCommand = new API_AT_RemoteCommand(frameId_, "IR", int2Hex(ioSampleRate));
        setRemoteAddressing(at_remoteCommand, networkAddr);
        at_remoteCommand->setRemoteCommandOption(API_AT_RemoteCommand::APPLY_CH);

        txrx_->sendMessage(at_remoteCommand->getFrame());

        delete at_remoteCommand;
        return incrementFrameID() - 0x1;
    }
}

// Send AT remote command method
unsigned char ZB_MonitoringAndControl::sendATCommand(string networkAddr,
                                                     string addr,
                                                     string atCommand,
                                                     string parameter,
                                                     API_AT_RemoteCommand::RemoteCommandOption option)
{
    if(networkAddr.compare("") == 0 && addr.compare("") == 0){

        API_AT_Command *atLocalCommand = new API_AT_Command(frameId_, atCommand, parameter);
        txrx_->sendMessage(atLocalCommand->getFrame());

        delete atLocalCommand;
        return incrementFrameID() - 0x1;
    }
    else{
        API_AT_RemoteCommand *at_remoteCommand;
        at_remoteCommand = new API_AT_RemoteCommand(frameId_, atCommand, parameter);
        setRemoteAddressing(at_remoteCommand, networkAddr, addr);
        at_remoteCommand->setRemoteCommandOption(option);

        txrx_->sendMessage(at_remoteCommand->getFrame());

        delete at_remoteCommand;
        return incrementFrameID() - 0x1;
    }
}

// writeChanges method
unsigned char ZB_MonitoringAndControl::writeChanges(string networkAddr)
{
    if(networkAddr.compare("") == 0){

        API_AT_Command *atCommand = new API_AT_Command(frameId_, "WR", "");

        txrx_->sendMessage(atCommand->getFrame());

        delete atCommand;
        return incrementFrameID() - 0x1;
    }
    else{

        return sendATCommand(networkAddr, "", "WR");
        /*API_AT_RemoteCommand *at_remoteCommand;
        at_remoteCommand = new API_AT_RemoteCommand(frameId_, "WR", "");
        setRemoteAddressing(networkAddr, at_remoteCommand);
        at_remoteCommand->setRemoteCommandOption(API_AT_RemoteCommand::APPLY_CH);

        txrx_->sendMessage(at_remoteCommand->getFrame());

        delete at_remoteCommand;
        return incrementFrameID() - 0x1;*/
    }
}

// start monitoring method
void ZB_MonitoringAndControl::startMonitoring()
{
    txrx_->start();
    run_ = true;
    openThread();
}

// stop monitoring method
void ZB_MonitoringAndControl::stopMonitoring()
{
    run_ = false;
    //txrx_->stop();
}

// job threaded method
void ZB_MonitoringAndControl::job()
{
    string message = "";
    API_Frame* frame;

    cout << "DEBUG: Running MC Job..." << endl;

    while(run_)
    {
        txrx_->accessMessagePool(message);

        if(message.compare("") != 0){
            /*cout << "DEBUG: Message received: ";
            for(unsigned int x = 0; x < message.length(); x++){
                cout << hex << (int)(unsigned char)message[x] << " ";
            }
            cout << endl;*/
            switch((unsigned char)message[3])
            {

                case API_Frame::AT_COMMAND_RESPONSE:
                {
                    API_AT_CommandResponse* at_response;

                    cout << "It's an AT_COMMAND_RESPONSE" << endl;
                    frame = new API_AT_CommandResponse();
                    frame->parseFrame(message);

                    at_response = dynamic_cast<API_AT_CommandResponse*>(frame);

                    cout << "Command Status: ";
                    processATCommandStatus(at_response);

                    delete at_response;
                    break;
                }
                case API_Frame::MODEM_STATUS:

                    cout << "It's a MODEM STATUS" << endl;
                    break;

                case API_Frame::TX_STATUS:

                    cout << "It's TRANSMISSION STATUS" << endl;
                    break;

                case API_Frame::RX_PACKET:

                    cout << "It's a RECEIVE PACKET" << endl;
                    frame = new API_Receive_Packet();
                    frame->parseFrame(message);
                    break;

                case API_Frame::EADDRESS_RX_PACKET:

                    cout << "It's EXPLICIT RX INDICATOR" << endl;
                    break;

                case API_Frame::IO_RX_SAMPLE:
                {
                    bool newNodeFound = true;
                    string sample = "";
                    API_IO_Sample* io_sample;

                    cout << "It's an IO Sample" << endl;
                    frame = new API_IO_Sample();
                    frame->parseFrame(message);

                    io_sample = dynamic_cast<API_IO_Sample*>(frame);
                    sample = io_sample->getAnalogSamples().begin()->second;

                    cout << endl << "Source Network Address:" << hex;
                    for(unsigned int x = 0; x < io_sample->getSourceNetworkAddress().size(); x++){
                        cout << (int)(unsigned char)io_sample->getSourceNetworkAddress()[x] << " ";
                    }
                    cout << endl;

                    if(auto_update_nodes_){

                        if(!nodeList_.empty()){

                            //cout << "Comparing..." << endl;
                            for(unsigned int i = 0; i < nodeList_.size(); i++){

                                /*cout << *networkNodes[i];

                                cout << endl << "Source Network Address:" << hex;
                                for(unsigned int x = 0; x < io_sample->getSourceNetworkAddress().size(); x++){
                                    cout << (int)(unsigned char)io_sample->getSourceNetworkAddress()[x] << " ";
                                }

                                cout << dec << endl;*/

                                if(nodeList_[i]->getNetworkAddr().compare(io_sample->getSourceNetworkAddress()) == 0){

                                    newNodeFound = false;
                                    nodeSample_map_[nodeList_[i]->getNodeIdent()] = io_sample;
                                    cout << "DEBUG: IO Frame length: " << io_sample->getLength() << endl;
                                    //cout << "From map:" << endl << "\t";
                                    /*map<string, API_IO_Sample*>::iterator it = nodeSample_map_.find("END POINT");
                                    if (it != nodeSample_map_.end()){

                                        cout << it->second->getLength() << endl;
                                    }
                                    break;*/
                                    cout << "Source: " << nodeList_[i]->getNodeIdent() << endl;
                                }
                            }

                            // I'll have to check the NI from the sourceNetworkAddress and only then
                            // call the discoverNetworkNodes with the NI response as the parameter.
                            if(newNodeFound){

                                cout << "DEBUG: New node found!..." << endl;

                                map<string, unsigned char>::iterator it = internalAT_frameId_map_.find(io_sample->getSourceNetworkAddress());
                                // Check if there's already a request for Node identification for this source network address.
                                if(it != internalAT_frameId_map_.end()){

                                    cout << "DEBUG: AT NI command already issued for this address." << endl;

                                    Resumed_AT_Response atResponse;

                                    // Get the resumed AT response regarding the parameterized frameId (it->second)
                                    cout << "DEBUG: Checking for AT response..." << flush;
                                    if(retrieveCommandsResponseBuffer(it->second, atResponse)){
                                        // Issue AT ND (Node Discovery) command for the retrieved NI value from the node.
                                        cout << "FOUND!" << endl << "DEBUG: Issuing AT ND for " << atResponse.s_value << endl;
                                        discoverNetworkNodes(atResponse.s_value);
                                    }
                                    else
                                        cout << "NOT FOUND!" << endl;
                                }
                                // If no call to AT NI has been issued for this node, send it and add an entry
                                // to the internal map.
                                else{
                                    cout << "DEBUG: Issuing AT NI command..." << endl;
                                    //internalAT_frameId_map_[io_sample->getSourceNetworkAddress()] = setNodeIdentifier(io_sample->getSourceNetworkAddress());
                                    internalAT_frameId_map_[io_sample->getSourceNetworkAddress()] = sendATCommand(io_sample->getSourceNetworkAddress(),
                                                                                                                  io_sample->getSourceAddress(),
                                                                                                                  "NI");
                                }
                            }
                        }
                        else
                            discoverNetworkNodes();
                    }


                    //getTemperatureCelsius((unsigned char)sample[0]*0x100 + (unsigned char)sample[1]);

                    //delete io_sample;
                    break;
                }
                case API_Frame::NODE_IDENT:

                    cout << "It's NODE IDENT" << endl;
                    break;

                case API_Frame::AT_REMOTE_RESPONSE:
                {

                    API_AT_RemoteCommandResponse* at_response;

                    cout << "It's  a AT REMOTE RESPONSE" << endl;
                    frame = new API_AT_RemoteCommandResponse();
                    frame->parseFrame(message);

                    at_response = dynamic_cast<API_AT_RemoteCommandResponse*>(frame);

                    cout << "Command Status: ";
                    processATCommandStatus(at_response);

                    delete at_response;
                    break;
                }

                default:
                    cout << "Another message type..." << endl;
                    break;
            }
            message = "";
        }

        sleep (1);
    }
}

// Process AT command status method
void ZB_MonitoringAndControl::processATCommandStatus(API_AT_CommandResponse* at_response)
{
    switch(at_response->getCommandStatus())
    {
        // If OK status has been received from an issued AT command, then the parameter value should be shown (if any).
        case API_AT_CommandResponse::OK:
        {

            //unsigned int bandwith = 0;
            API_AT_CommandResponse::ATCommands atCommand = API_AT_CommandResponse::UNKNOWN_COMMAND;
            cout << "OK" << endl;
            cout << "AT Command: " << at_response->getATCommand() << endl;


            /* Depending on the command issued, the parameter value should get different treatment. For this particular case
             * the AT command was BD, so BD parameter ranges apply.
             */
            atCommand = API_AT_CommandResponse::ATCommands(((unsigned char)at_response->getATCommand()[0]*0x100) + (unsigned  char)at_response->getATCommand()[1]);

            switch(atCommand)
            {
                case API_AT_CommandResponse::SERIAL_CRE_INTERFACE_DATA_RATE:
                {
                    Resumed_AT_Response resumedATResponse;
                    resumedATResponse.commandStatus = API_AT_CommandResponse::CommandStatus(at_response->getCommandStatus());
                    resumedATResponse.parameterValueType = UNSIGNED_INT;
                    resumedATResponse.parameterValue.ui_value = at_response->get_ATBD_Value();


                    commandsResponse_buffer_[at_response->getFrameId()] = resumedATResponse;

                    break;
                }

                case API_AT_CommandResponse::EXEC_CRE_NODE_DISCOVERY:
                {
                    updateNodeList(new ZB_Node(at_response->getParameterValue()));

                    break;
                }

                case API_AT_CommandResponse::ADDR_CRE_NODE_IDENTIFIER:
                {
                    Resumed_AT_Response resumedATResponse;
                    resumedATResponse.commandStatus = API_AT_CommandResponse::CommandStatus(at_response->getCommandStatus());
                    resumedATResponse.parameterValueType = STRING;
                    resumedATResponse.s_value = at_response->getParameterValue();

                    commandsResponse_buffer_[at_response->getFrameId()] = resumedATResponse;

                    cout << "DEBUG: Received response: ADDR_CRE_NODE_IDENTIFIER" << endl
                    << "Value: " << resumedATResponse.s_value << endl;

                    break;
                }

                default:

                    cout << "Parameter value: ";
                    for(unsigned int i = 0; i < at_response->getParameterValue().length(); i++){

                        cout << hex << (int)(unsigned char)at_response->getParameterValue()[i];
                    }

                    break;
            }

            cout << endl;

            break;
        }
        case API_AT_CommandResponse::ERROR:
            cout << "ERROR" << endl;
            cout << "AT Command: " << at_response->getATCommand() << endl;
            break;

        case API_AT_CommandResponse::INVALID_COMMAND:
            cout << "INVALID COMMAND" << endl;
            break;

        case API_AT_CommandResponse::INVALID_PARAMETER:
            cout << "INVALID PARAMETER" << endl;
            break;

        case API_AT_CommandResponse::TX_FAILURE:
            cout << "TX_FAILURE" << endl;
            break;

        default:
            cout << "UNKNOWN" << endl;
            break;
    }
}

// incrementFrameID method
unsigned char ZB_MonitoringAndControl::incrementFrameID()
{
    if(frameId_ == 0xFF){
        frameId_ = 0x1;
        return 0xFF;
    }
    else{
        frameId_ += 0x1;
        return frameId_;
    }
}

// int2Hex method
string ZB_MonitoringAndControl::int2Hex(unsigned int number)
{
    string hexNumber = "", aux = "";

	while(number > 0){

		aux.append(1, (number & 0xFF));
		aux.append(hexNumber);
		hexNumber = aux;
		aux = "";
		number = number >> 8;
	}

	return hexNumber;
}

// updateNodeList method
void ZB_MonitoringAndControl::updateNodeList(ZB_Node* node)
{
    bool found = false, updated = false;

    for (unsigned int i = 0; i < nodeList_.size(); i++){

        if (nodeList_[i]->getNetworkAddr() == node->getNetworkAddr()){

            found = true;
            if(*nodeList_[i] == *node)
                break;

            else{
                nodeList_.erase(nodeList_.begin() + i);
                nodeList_.push_back(node);
                updated = true;
                break;
            }
        }
    }

    if(!found)
        nodeList_.push_back(node);
    else if(found && !updated)
        delete node;
}

// biCount method
unsigned int ZB_MonitoringAndControl::bitCount(unsigned int number)
{
    unsigned int count = 0;

	while(number > 0)
	{
		number = number >> 1;
		count++;
	}

	return count;
}

// setRemoteAddressing method
bool ZB_MonitoringAndControl::setRemoteAddressing(API_AT_RemoteCommand* remoteCommand, string nodeIdent, string address)
{
    bool set = false;

    //cout << "DEBUG: " << remoteCommand->getATCommand() << endl;

    lock();
    {
        for(unsigned int i = 0; i < nodeList_.size(); i++){

            if(nodeList_[i]->getNodeIdent().compare(nodeIdent) == 0 ||
               nodeList_[i]->getNetworkAddr().compare(nodeIdent) == 0 ||
               string(nodeList_[i]->getSerialNumberHigh() + nodeList_[i]->getSerialNumberLow()).compare(nodeIdent) == 0){

                 remoteCommand->setDestinationAddress(string(nodeList_[i]->getSerialNumberHigh() + nodeList_[i]->getSerialNumberLow()));
                 remoteCommand->setDestinationNetworkAddress(nodeList_[i]->getNetworkAddr());
                 set = true;
                 break;
            }
        }
    }
    unlock();

    // If current node ins't yet on the node list, use only the (valid?) network address
    if (!set && nodeIdent.size() == 2 && address.size() == 8){
        remoteCommand->setDestinationAddress(address);
        remoteCommand->setDestinationNetworkAddress(nodeIdent);
        set = true;
    }

    return set;
}
