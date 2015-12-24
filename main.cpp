#include <iostream>

#include "gpiowork.h"
#include "control_module.h"
#include "gyrowork.h"
#include "compasswork.h"
#include "barowork.h"

using namespace std;

int main()
{
	cout << "Hello World!" << endl;

	gpio::gpiowork gpiowrk;
	control_module cm;
	gyrowork gwork;
	compasswork cwork;
	barowork bwork;

	cm.sigctrl.connect(boost::bind(&gpio::gpiowork::handler_signal, &gpiowrk));

	gwork.set_control_module(&cm);
	gpiowrk.set_control_module(&cm);
	cwork.set_control_module(&cm);
	bwork.set_control_module(&cm);

	boost::thread thr_wrk(boost::bind(&gpio::gpiowork::run, &gpiowrk));
	boost::thread thr_cm(boost::bind(&control_module::run, &cm));
	boost::thread thr_gwrk(boost::bind(&gyrowork::run, &gwork));
	boost::thread thr_cwrk(boost::bind(&compasswork::run, &cwork));
	boost::thread thr_bwrk(boost::bind(&barowork::run, &bwork));

	gpiowrk.open_pin(11, 1.5, 50);
	gpiowrk.open_pin(12, 1.1, 500);

	//_msleep(2000);
	//cm.set_address("0.0.0.0", 8888);

	thr_bwrk.join();
	thr_cwrk.join();
	thr_gwrk.join();
	thr_cm.join();
	thr_wrk.join();

	return 0;
}

