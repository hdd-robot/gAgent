#include <vector>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdio.h>

#include "FipaInterface_impl.h"


#include <sys/wait.h>

MTS_i::MTS_i(){}

MTS_i::~MTS_i(void){}

void MTS_i::message(const ::FIPA::FipaMessage& aFipaMessage){

	std::cout << "message_recu  : " << std::endl;

}

