/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include "dataplane.h"


int main(int argc, char **argv) {
	
	DataPlane dp;
    thread t2(&DataPlane::updateTest, &dp);
    dp.run(argc,argv);
    t2.join();
	return 0;
}