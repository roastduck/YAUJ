#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <string>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include "abstractstubserver.h"
#include "config_daemon.h"

using namespace jsonrpc;

int runningCnt, totCnt, preserveCnt;
std::set<int> syncing;
std::multiset<int> boardingPass;

pthread_mutex_t cntLock = PTHREAD_MUTEX_INITIALIZER, syncLock = PTHREAD_MUTEX_INITIALIZER, cmdLock = PTHREAD_MUTEX_INITIALIZER;

Json::Value dumpCmd(const std::string &cmd, const std::string &dir)
{
#ifdef DEBUG
	std::clog << cmd << std::endl;
#endif
	pid_t child = fork();
	if (!child)
	{
		chdir(dir.c_str());
		int exitCode = system((cmd+" > yauj.res 2>yauj.log").c_str());
		if (exitCode)
		{
			FILE *e = fopen("yauj.log","r");
			char *b = new char [PIPE_READ_BUFF_MAX+1];
			b[fread(b,1,PIPE_READ_BUFF_MAX,e)]=0;
			fclose(e);
			Json::Value ret;
			ret["error"]=b;
			e = fopen("yauj.res","w");
			fputs(ret.toStyledString().c_str(),e);
			fclose(e);
			delete b;
		}
		exit(0);
	} else
	{
		waitpid(child, 0, 0);
#ifdef DEBUG
		std::clog << "done" << std::endl;
#endif
		FILE *res = fopen((dir+"/yauj.res").c_str(),"r");
		char *buff = new char [PIPE_READ_BUFF_MAX+1];
		buff[fread(buff,1,PIPE_READ_BUFF_MAX,res)]=0;
		if (!feof(res))
		{
			fclose(res);
			delete buff;
			throw std::string("[ERROR] PIPE_READ_BUFF_MAX exceeded");
		}
		fclose(res);
		Json::Value ret;
		Json::Reader reader;
		if (!reader.parse(buff,ret))
		{
			delete buff;
			throw std::string("[ERROR] return value is not JSON format");
		}
		delete buff;
		return ret;
	}
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
			ss << runPath << "/" << _totCnt_;
			std::string runDir = ss.str();
			pthread_mutex_lock(&cmdLock);
#ifdef DEBUG
			std::clog << "mkdir -p "+runDir << std::endl;
#endif
			system(("mkdir -p "+runDir).c_str());
#ifdef DEBUG
			std::clog << "rm -r "+runDir+"/*" << std::endl;
#endif
			system(("rm -r "+runDir+"/*").c_str());
			pthread_mutex_unlock(&cmdLock);
			try
			{
				ss.str("");
				ss << "cp " + dataPath + "/" << pid << "/* " << runDir;
				pthread_mutex_lock(&syncLock);
				if (syncing.count(pid)) pthread_mutex_unlock(&syncLock), throw std::string("data updated when copying files.");
#ifdef DEBUG
				std::clog << ss.str() << std::endl;
#endif
				pthread_mutex_lock(&cmdLock), system(ss.str().c_str()), pthread_mutex_unlock(&cmdLock);
				pthread_mutex_unlock(&syncLock);
				ss.str("");
				ss << "cp " + sourcePath + "/" << sid/10000 << '/' << sid%10000 << "/* " << runDir;
#ifdef DEBUG
				std::clog << ss.str() << std::endl;
#endif
				pthread_mutex_lock(&cmdLock), system((ss.str()).c_str()), pthread_mutex_unlock(&cmdLock);
				ss.str("");
				ss << "./yauj_judge run";
				for (Json::ValueIterator i=submission.begin(); i!=submission.end(); i++)
					ss << " " << (*i)["language"].asString() << " " << (*i)["source"].asString();
				ret=dumpCmd(ss.str(),runDir);
#ifndef DEBUG
				pthread_mutex_lock(&cmdLock), system(("rm -r "+runDir).c_str()), pthread_mutex_unlock(&cmdLock);
#endif
			} catch (std::string &e)
			{
#ifdef DEBUG
				std::clog << " run: catched " << e << std::endl;
#endif
				pthread_mutex_lock(&cntLock);
				runningCnt--, preserveCnt--;
				pthread_mutex_unlock(&cntLock);
				ret["error"] = e;
				return ret;
			}
			pthread_mutex_lock(&cntLock);
			runningCnt--, preserveCnt--;
			pthread_mutex_unlock(&cntLock);
			return ret;
		}
		
		/*virtual Json::Value loadConf(int pid)
		{
			pthread_mutex_lock(&cntLock);
			const int _totCnt_ = ++totCnt;
			pthread_mutex_unlock(&cntLock);
			std::ostringstream ss;
			ss << runPath << "/" << _totCnt_;
			try
			{
				return dumpCmd("./yauj_judge loadconf",ss.str());
			} catch (std::string &e)
			{
				Json::Value ret;
				ret["error"] = e;
				return ret;
			}
		}*/
		
		virtual Json::Value judgeStatus()
		{
			Json::Value ret;
			ret["runningCnt"]=runningCnt;
			ret["totDumpCmdCnt"]=totCnt;
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
			if (preserveCnt >= MAX_RUN) return -1;
			pthread_mutex_lock(&cntLock);
			preserveCnt++;
			pthread_mutex_unlock(&cntLock);
			
			int exitCode;
			pid_t child = fork();
			if (!child)
			{
				std::ostringstream s;
				s << "mkdir -p " << sourcePath << '/' << sid/10000;
				system(s.str().c_str());
				s.str("");
				s << "rsync -e 'ssh -c arcfour' -rz -W --del "WEB_SERVER":" << sourcePath << '/' << sid/10000 << '/' << sid%10000 << ' ' << sourcePath << '/' << sid/10000;
				exit(system(s.str().c_str()));
			}
			waitpid(child,&exitCode,0);
			if (!WIFEXITED(exitCode)||WEXITSTATUS(exitCode)) return -1;
			
			int ret;
			pthread_mutex_lock(&cntLock);
			ret = rand();
			boardingPass.insert(ret);
			pthread_mutex_unlock(&cntLock);
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
#ifdef DEBUG
			std::clog << " sync: unlocked syncLock" << std::endl;
#endif

			int exitCode;
			pid_t child = fork();
			if (!child)
			{
				int ret;
				system(("mkdir -p "+dataPath).c_str());
				std::ostringstream s;
				s << "rsync -e 'ssh -c arcfour' -crz --del "WEB_SERVER":" << dataPath << '/' << pid << ' ' << dataPath << ". >/dev/null";
				if (system(s.str().c_str())) exit(1);
				s.str("");
				s << "make -C " << dataPath << '/' << pid << " >/dev/null 2>&1";
				if (system("make >/dev/null 2>&1")) exit(1);
				exit(0);
			}
			waitpid(child,&exitCode,0);
			pthread_mutex_lock(&syncLock), syncing.erase(pid), pthread_mutex_unlock(&syncLock);
#ifdef DEBUG
			std::clog << " synced" << std::endl;
#endif
			return (WIFEXITED(exitCode)&&!WEXITSTATUS(exitCode))?"failed":"success";
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

