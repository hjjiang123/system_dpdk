#include <cstdio>
#include "server.h"

int main(int argc, char **argv)
{
	// FILE* logFile = fopen("./log/1", "w");
	// if (logFile == nullptr) {
	// 	perror("Failed to open log file");
	// 	return 1;
	// }
	
	init(argc, argv);
	// configureNumCores(5);
	
	// rte_openlog_stream(logFile);
	int ret = init_port(0);
	if (ret < 0) {
		rte_exit(EXIT_FAILURE, "Cannot init port %d\n", 0);
	}
	initPluginRuntimeList();
	runserver();

	// std::thread t2(updateTest);
	run();
	// t2.join();

	// fclose(logFile);
	return 0;
}
