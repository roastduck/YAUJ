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
#include <syslog.h>
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
	pid_t child = fork();
	if (!child)
	{
#ifdef DEBUG
		std::clog << "chdir to " << dir << std::endl;
#endif
		chdir(dir.c_str());
#ifdef DEBUG
		std::clog << cmd << std::endl;
#endif
		int exitCode=system((cmd+" > yauj.res 2>yauj.log").c_str());
		if (!WIFEXITED(exitCode))
			syslog(LOG_ERR, "failed to run command");
		exit(WEXITSTATUS(exitCode));
	} else
	{
		int exitCode;
		waitpid(child, &exitCode, 0);
		
#ifdef DEBUG
		std::clog << "uoj_run done with exit code " << WEXITSTATUS(exitCode) << std::endl;
#endif
		
		bool error = (!WIFEXITED(exitCode) || WEXITSTATUS(exitCode));
		if (error)
			syslog(LOG_ERR, "%s", ("An error. cmd="+cmd+" dir="+dir).c_str());
		FILE *res = fopen((dir+(error?"/yauj.log":"/yauj.res")).c_str(),"r");
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
		if (error)
			ret["error"] = buff;
		else
		{
			Json::Reader reader;
			if (!reader.parse(buff,ret))
			{
				delete buff;
				throw std::string("[ERROR] return value is not JSON format");
			}
		}
		delete buff;

#ifdef DEBUG
		std::clog << "yauj_judge result: " << ret << std::endl;
#endif

		return ret;
	}
}

class Server : public AbstractStubServer
{
	const std::string webServer, dataPath, runPath, sourcePath;
	
	public :
		Server(AbstractServerConnector &connector, serverVersion_t type, const std::string &_webServer, const std::string &_dataPath, const std::string &_runPath, const std::string &_sourcePath)
			: webServer(_webServer), dataPath(_dataPath), runPath(_runPath), sourcePath(_sourcePath), AbstractStubServer(connector,type) {}
		
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
				for (Json::Value::const_iterator i=submission.begin(); i!=submission.end(); i++)
					ss << " " << (*i)["language"].asString() << " " << (*i)["source"].asString();
				ret=dumpCmd(ss.str(),runDir);
#ifndef NCLEAN
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
				if (webServer=="127.0.0.1" || webServer=="localhost") exit(0);
				std::ostringstream s;
				s << "mkdir -p " << sourcePath << '/' << sid/10000;
				system(s.str().c_str());
				s.str("");
				s << "rsync -e 'ssh -c arcfour' -rz -W --del " << webServer << ":" << sourcePath << '/' << sid/10000 << '/' << sid%10000 << ' ' << sourcePath << '/' << sid/10000;
				int exitCode=system(s.str().c_str());
				if (!WIFEXITED(exitCode))
					syslog(LOG_ERR, "failed to run rsync");
				exit(WEXITSTATUS(exitCode));
			}
			waitpid(child,&exitCode,0);
			if (!WIFEXITED(exitCode)||WEXITSTATUS(exitCode))
			{
				pthread_mutex_lock(&cntLock);
				preserveCnt--;
				pthread_mutex_unlock(&cntLock);
				return -1;
			}
			
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
				std::ostringstream s;
				int ret;
				if (webServer!="127.0.0.1" && webServer!="localhost")
				{
					system(("mkdir -p "+dataPath).c_str());
					s << "rsync -e 'ssh -c arcfour' -crz --del " << webServer << ":" << dataPath << '/' << pid << ' ' << dataPath << " >/dev/null";
					if (system(s.str().c_str()))
					{
						syslog(LOG_ERR,"sync : rsync failed. pid=%d",pid);
						exit(1);
					}
				}
				s.str("");
				s << "make -i -B -C " << dataPath << '/' << pid << " > " << dataPath << '/' << pid << "/make.log 2>&1";
				if (system(s.str().c_str())) 
					syslog(LOG_WARNING,"sync : make failed. pid=%d",pid);
				exit(0);
			}
			waitpid(child,&exitCode,0);
			pthread_mutex_lock(&syncLock), syncing.erase(pid), pthread_mutex_unlock(&syncLock);
#ifdef DEBUG
			std::clog << " synced" << std::endl;
#endif
			if (WIFEXITED(exitCode)&&WEXITSTATUS(exitCode)==2) return "error";
			return (WIFEXITED(exitCode)&&!WEXITSTATUS(exitCode))?"success":"failed";
		}
};

Json::Value config;

bool readConf()
{
	Json::Reader reader;
	char buff[CONFIG_BUFF_MAX+1];
	FILE *f = fopen("/etc/yauj/daemon.json","r");
	buff[fread(buff,1,CONFIG_BUFF_MAX,f)]=0;
	if (!feof(f)) return fclose(f), false;
	fclose(f);
	if (!reader.parse(buff,config)) return false;
	return true;
}

int main()
{
	openlog("yauj_daemon", LOG_PID, LOG_USER);
	srand(time(0));
	if (!readConf())
	{
		syslog(LOG_ERR, "Can't read configuration");
		return 1;
	}
	for (Json::Value::iterator i=config.begin(); i!=config.end(); i++)
	{
		(new Server(
				  *(new HttpServer((*i)[std::string("port")].asInt())),
				  JSONRPC_SERVER_V1V2,
				  (*i)[std::string("webServer")].asString(),
				  (*i)[std::string("dataPath")].asString(),
				  (*i)[std::string("runPath")].asString(),
				  (*i)[std::string("sourcePath")].asString()
		))->StartListening();
		syslog(LOG_INFO, "Listening Started on Port %d", (*i)[std::string("port")].asInt());
	}
	pause();
}

