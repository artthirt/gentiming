#include <iostream>

#include "gpiowork.h"
#include "control_module.h"

using namespace std;

int main()
{
	cout << "Hello World!" << endl;

	gpio::gpiowork wrk;
	control_module cm;

	boost::thread thr_wrk(boost::bind(&gpio::gpiowork::run, &wrk));
	boost::thread thr_cm(boost::bind(&control_module::run, &cm));

	wrk.open_pin(1, 1.5, 50);
	wrk.open_pin(2, 1.1, 500);

	//_msleep(2000);
	//cm.set_address("0.0.0.0", 8888);

	thr_cm.join();
	thr_wrk.join();

	return 0;
}

