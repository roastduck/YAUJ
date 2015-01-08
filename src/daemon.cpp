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
	FILE *res = popen((cmd+" 2>yauj.log").c_str(),"r");
	char *buff = new char [PIPE_READ_BUFF_MAX+1];
	buff[fread(buff,1,PIPE_READ_BUFF_MAX,res)]=0;
	if (!feof(res)) throw std::string("[ERROR] PIPE_READ_BUFF_MAX exceeded");
	int retcode = pclose(res);
	if (retcode)
	{
		FILE *e = fopen("yauj.log","r");
		char *b = new char [PIPE_READ_BUFF_MAX+1];
		b[fread(b,1,PIPE_READ_BUFF_MAX,e)]=0;
		fclose(e);
		std::string err = b;
		delete buff;
		delete b;
		throw err;
	}
	Json::Value ret;
	if (!reader.parse(buff,ret)) throw std::string("[ERROR] return value is not JSON format");
	delete buff;
	return ret;
}

class Server : public AbstractStubServer
{
	int runningCnt, totCnt;
	std::string dataPath, runPath, sourcePath;
	
	public :
		Server(AbstractServerConnector &connector, serverVersion_t type, char *_dataPath, char *_runPath, char *_sourcePath)
			: runningCnt(0), totCnt(0), dataPath(_dataPath), runPath(_runPath), sourcePath(_sourcePath), AbstractStubServer(connector,type) {}
		
		virtual Json::Value run(int pid, int sid, const Json::Value &submission)
		{
			runningCnt++, totCnt++;
			Json::Value ret;
			try
			{
				std::ostringstream ss, ss2;
				char cwd[WD_BUFF_MAX];
				getcwd(cwd,WD_BUFF_MAX);
				ss << runPath << "/" << totCnt;
#ifdef DEBUG
				std::clog << ("mkdir -p "+ss.str()) << std::endl;
#endif
				system(("mkdir -p "+ss.str()).c_str());
#ifdef DEBUG
				std::clog << "chdir to " << ss.str() << std::endl;
#endif
				chdir(ss.str().c_str());
				ss.str("");
				ss << "cp " + dataPath + "/" << pid << "/* .";
#ifdef DEBUG
				std::clog << ss.str() << std::endl;
#endif
				system(ss.str().c_str());
				ss.str("");
				ss << "./yauj_judge run";
				ss2 << "cp " + sourcePath + "/" << sid << '/';
				for (Json::ValueIterator i=submission.begin(); i!=submission.end(); i++)
				{
					ss << " " << (*i)["language"].asString() << " " << (*i)["source"].asString();
					ss2 << (*i)["source"].asString() << ' ';
				}
#ifdef DEBUG
				std::clog << ss2.str()+"." << std::endl;
#endif
				system((ss2.str()+".").c_str());
				ret=dumpCmd(ss.str());
				chdir(cwd);
#ifndef DEBUG
				ss.str("");
				ss << "rm -r " << runPath << "/" << totCnt;
				system(ss.str().c_str());
#endif
			} catch (std::string &e)
			{
				runningCnt--;
				ret["error"] = e;
				return ret;
			}
			runningCnt--;
			return ret;
		}
		
		virtual Json::Value loadConf(int pid)
		{
			try
			{
				std::ostringstream ss;
				ss << dataPath + "/" << pid << "/yauj_judge loadconf";
				return dumpCmd(ss.str());
			} catch (std::string &e)
			{
				Json::Value ret;
				ret["error"] = e;
				return ret;
			}
		}
		
		virtual Json::Value judgeStatus()
		{
			Json::Value ret;
			ret["runningCnt"]=runningCnt;
			ret["totCnt"]=totCnt;
			return ret;
		}
};

int ports[] = { LISTEN_PORT , 0 };
char dataPath[][256] = { DATA_PATH , "" }, runPath[][256] = { RUN_PATH , "" }, sourcePath[][256] = { SOURCE_PATH , ""};

int main()
{
	/*HttpServer httpserver(LISTEN_PORT);
	Server server(httpserver, JSONRPC_SERVER_V1V2);
	server.StartListening();
	std::clog << "Started Listening" << std::endl;*/
	for (int i=0; ports[i]; i++)
	{
		(new Server(*(new HttpServer(ports[i])),JSONRPC_SERVER_V1V2,dataPath[i],runPath[i],sourcePath[i]))->StartListening();
		std::clog << "Listening Started on Port " << ports[i] << std::endl;
	}
	std::cin.get();
}

