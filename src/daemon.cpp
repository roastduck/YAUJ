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
#ifdef DEBUG
	std::clog << cmd << std::endl;
#endif
	FILE *res = popen(cmd.c_str(),"r");
	char *buff = new char [PIPE_READ_BUFF_MAX+1];
	buff[fread(buff,1,PIPE_READ_BUFF_MAX,res)]=0;
	if (!feof(res))
		std::cerr << "[ERROR] PIPE_READ_BUFF_MAX exceeded" << std::endl;
	pclose(res);
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
		
		virtual Json::Value run(int pid, int sid, const Json::Value &submission)
		{
			runningCnt++, totCnt++;
			std::ostringstream ss, ss2;
			char cwd[WD_BUFF_MAX];
			getcwd(cwd,WD_BUFF_MAX);
			ss << RUN_PATH << "/" << totCnt;
#ifdef DEBUG
			std::clog << ("mkdir -p "+ss.str()) << std::endl;
#endif
			system(("mkdir -p "+ss.str()).c_str());
#ifdef DEBUG
			std::clog << "chdir to " << ss.str() << std::endl;
#endif
			chdir(ss.str().c_str());
			ss.str("");
			ss << "cp " DATA_PATH "/" << pid << "/* .";
#ifdef DEBUG
			std::clog << ss.str() << std::endl;
#endif
			system(ss.str().c_str());
			ss.str("");
			ss << "./yauj_judge run";
			ss2 << "cp " SOURCE_PATH "/" << sid << '/';
			for (Json::ValueIterator i=submission.begin(); i!=submission.end(); i++)
			{
				ss << " " << (*i)["language"].asString() << " " << (*i)["source"].asString();
				ss2 << (*i)["source"].asString() << ' ';
			}
#ifdef DEBUG
			std::clog << ss2.str()+"." << std::endl;
#endif
			system((ss2.str()+".").c_str());
			Json::Value ret=dumpCmd(ss.str());
			chdir(cwd);
#ifndef DEBUG
			ss.str("");
			ss << "rm -r " << RUN_PATH << "/" << totCnt;
			system(ss.str().c_str());
#endif
			runningCnt--;
			return ret;
		}
		
		virtual Json::Value loadConf(int pid)
		{
			std::ostringstream ss;
			ss << DATA_PATH"/" << pid << "/yauj_judge loadconf";
			return dumpCmd(ss.str());
		}
		
		virtual Json::Value judgeStatus()
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

