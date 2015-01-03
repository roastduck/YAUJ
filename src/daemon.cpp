#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include "abstractstubserver.h"
#include "config_daemon.h"

using namespace jsonrpc;

Json::Reader reader;

Json::Value dumpCmd(const std::string &cmd)
{
	FILE *res = popen(cmd.c_str(),"r");
	char *buff = new char [PIPE_READ_BUFF_MAX+1];
	buff[fread(buff,1,PIPE_READ_BUFF_MAX,res)]=0;
	fclose(res);
	if (!feof(res))
		std::cerr << "[ERROR] PIPE_READ_BUFF_MAX exceeded" << std::endl;
	Json::Value ret;
	if (!reader.parse(buff,ret))
		std::cerr << "[ERROR] return value is not JSON format" << std::endl;
	delete buff;
	return ret;
}

class Server : public AbstractStubServer
{
	int runningCnt, totCnt;
	
	public :
		Server(AbstractServerConnector &connector, serverVersion_t type)
			: runningCnt(0), totCnt(0), AbstractStubServer(connector,type) {}
		
		virtual Json::Value run(int pid, const Json::Value &submission)
		{
			runningCnt++, totCnt++;
			std::ostringstream ss;
			char cwd[WD_BUFF_MAX];
			getwd(cwd);
			ss << RUN_PATH << "/" << totCnt;
			system(("mkdir "+ss.str()).c_str());
			chdir(ss.str().c_str());
			ss.str("");
			ss << "cp " DATA_PATH "/" << pid << "/* .";
			system(ss.str().c_str());
			ss.str("./yauj_judger run");
			for (Json::ValueIterator i=submission.begin(); i!=submission.end(); i++)
				ss << " " << (*i)["language"].asString() << " " << (*i)["source"];
			Json::Value ret=dumpCmd(ss.str());
			chdir(cwd);
			runningCnt--;
			return ret;
		}
		
		virtual Json::Value loadConf(int pid)
		{
			std::ostringstream ss;
			ss << DATA_PATH"/" << pid << "/yauj_judger loadconf";
			return dumpCmd(ss.str());
		}
		
		virtual Json::Value judgerStatus()
		{
			Json::Value ret;
			ret["runningCnt"]=runningCnt;
			ret["totCnt"]=totCnt;
			return ret;
		}
};

int main()
{
	HttpServer httpserver(LISTEN_PORT);
	Server server(httpserver, JSONRPC_SERVER_V1V2);
	server.StartListening();
	std::clog << "Started Listening" << std::endl;
	std::cin.get();
}

