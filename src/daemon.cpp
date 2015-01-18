#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <string>
#include <sstream>
#include <iostream>
#include <pthread.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include "abstractstubserver.h"
#include "config_daemon.h"

using namespace jsonrpc;

int runningCnt, totCnt, preserveCnt;
std::set<int> syncing;
std::multiset<int> boardingPass;

pthread_mutex_t cntLock = PTHREAD_MUTEX_INITIALIZER, syncLock = PTHREAD_MUTEX_INITIALIZER;

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
	Json::Reader reader;
	if (!reader.parse(buff,ret)) throw std::string("[ERROR] return value is not JSON format");
	delete buff;
	return ret;
}

class Server : public AbstractStubServer
{
	const std::string dataPath, runPath, sourcePath;
	
	public :
		Server(AbstractServerConnector &connector, serverVersion_t type, char *_dataPath, char *_runPath, char *_sourcePath)
			: dataPath(_dataPath), runPath(_runPath), sourcePath(_sourcePath), AbstractStubServer(connector,type) {}
		
		virtual Json::Value run(int key, int pid, int sid, const Json::Value &submission)
		{
#ifdef DEBUG
			std::clog << "run" << std::endl;
			std::clog << " pid=" << pid << " sid=" << sid << " key=" << key << std::endl;
#endif
			pthread_mutex_lock(&cntLock);
			if (!boardingPass.count(key))
			{
				pthread_mutex_unlock(&cntLock);
				Json::Value ret;
				ret["error"] = "not preserved";
				return ret;
			}
			boardingPass.erase(boardingPass.find(key));
			runningCnt++, totCnt++;
			const int _totCnt_ = totCnt;
			pthread_mutex_unlock(&cntLock);
			Json::Value ret;
			std::ostringstream ss;
			char cwd[WD_BUFF_MAX];
			getcwd(cwd,WD_BUFF_MAX);
			ss << runPath << "/" << _totCnt_;
#ifdef DEBUG
			std::clog << ("mkdir -p "+ss.str()) << std::endl;
#endif
			system(("mkdir -p "+ss.str()).c_str());
#ifdef DEBUG
			std::clog << "chdir to " << ss.str() << std::endl;
#endif
			chdir(ss.str().c_str());
#ifdef DEBUG
			std::clog << "rm -r *" << std::endl;
#endif
			system("rm -r *");
			try
			{
				ss.str("");
				ss << "cp " + dataPath + "/" << pid << "/* .";
#ifdef DEBUG
				std::clog << ss.str() << std::endl;
#endif
				pthread_mutex_lock(&syncLock);
				if (syncing.count(pid)) throw pthread_mutex_unlock(&syncLock), std::string("data updated when copying files.");
				system(ss.str().c_str());
				pthread_mutex_unlock(&syncLock);
				ss.str("");
				ss << "cp " + sourcePath + "/" << sid << "/* .";
#ifdef DEBUG
				std::clog << ss.str() << std::endl;
#endif
				system((ss.str()).c_str());
				ss.str("");
				ss << "./yauj_judge run";
				for (Json::ValueIterator i=submission.begin(); i!=submission.end(); i++)
					ss << " " << (*i)["language"].asString() << " " << (*i)["source"].asString();
				ret=dumpCmd(ss.str());
				chdir(cwd);
#ifndef DEBUG
				ss.str("");
				ss << "rm -r " << runPath << "/" << _totCnt_;
				system(ss.str().c_str());
#endif
			} catch (std::string &e)
			{
				chdir(cwd);
				pthread_mutex_lock(&cntLock);
				runningCnt--;
				pthread_mutex_unlock(&cntLock);
				ret["error"] = e;
				return ret;
			}
			pthread_mutex_lock(&cntLock);
			runningCnt--, preserveCnt--;
			pthread_mutex_unlock(&cntLock);
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
			ret["preserveCnt"]=preserveCnt;
			ret["boardingPass"]=Json::Value();
			pthread_mutex_lock(&cntLock);
			for (std::multiset<int>::iterator i=boardingPass.begin(); i!=boardingPass.end(); i++)
				ret["boardingPass"].append(*i);
			pthread_mutex_unlock(&cntLock);
			return ret;
		}

		virtual int preserve(int sid)
		{
#ifdef DEBUG
			std::clog << "preserve" << std::endl;
#endif
			// MAKE SURE YOU DON'T REJUDGE A RUNNING SUBMISSION.
			int ret;
			if (preserveCnt >= MAX_RUN) return -1;
			pthread_mutex_lock(&cntLock);
			preserveCnt++;
			ret = rand();
			boardingPass.insert(ret);
			pthread_mutex_unlock(&cntLock);
			std::ostringstream s;
			s << "rsync -e 'ssh -c arcfour' -rz -W --del "WEB_SERVER":" << sourcePath << '/' << sid << ' ' << sourcePath;
			system(s.str().c_str());
			return ret;
		}
	
		virtual bool cancel(int key)
		{
#ifdef DEBUG
			std::clog << "cancel" << std::endl;
#endif
			bool ret = false;
			pthread_mutex_lock(&cntLock);
			if (boardingPass.count(key))
				ret = true, boardingPass.erase(boardingPass.find(key));
			preserveCnt--;
			pthread_mutex_unlock(&cntLock);
			return ret;
		}
		
		virtual std::string sync(int pid)
		{
#ifdef DEBUG
			std::clog << "sync" << std::endl;
#endif
			pthread_mutex_lock(&syncLock);
			if (syncing.count(pid)) return pthread_mutex_unlock(&syncLock), "syncing";
			syncing.insert(pid);
			pthread_mutex_unlock(&syncLock);
			char cwd[WD_BUFF_MAX];
			getcwd(cwd,WD_BUFF_MAX);
			system(("mkdir -p "+dataPath).c_str());
			chdir(dataPath.c_str());
			std::ostringstream s;
			s << "rsync -e 'ssh -c arcfour' -crz --del "WEB_SERVER":" << dataPath << '/' << pid << " . >/dev/null";
			int ret = system(s.str().c_str());
			if (ret)
				return chdir(cwd), pthread_mutex_lock(&syncLock), syncing.erase(pid), pthread_mutex_unlock(&syncLock), "failed";
			s.str("");
			s << pid;
			chdir(s.str().c_str());
			ret = system("make >/dev/null 2>&1");
			if (ret)
				return chdir(cwd), pthread_mutex_lock(&syncLock), syncing.erase(pid), pthread_mutex_unlock(&syncLock), "failed";
			pthread_mutex_lock(&syncLock), syncing.erase(pid), pthread_mutex_unlock(&syncLock);
			chdir(cwd);
			return "success";
		}
};

int ports[] = { LISTEN_PORT , 0 };
char dataPath[][256] = { DATA_PATH , "" }, runPath[][256] = { RUN_PATH , "" }, sourcePath[][256] = { SOURCE_PATH , ""};

int main()
{
	srand(time(0));
	for (int i=0; ports[i]; i++)
	{
		(new Server(*(new HttpServer(ports[i])),JSONRPC_SERVER_V1V2,dataPath[i],runPath[i],sourcePath[i]))->StartListening();
		std::clog << "Listening Started on Port " << ports[i] << std::endl;
	}
	std::cin.get();
}

